/*
 * exposewidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "exposewidget.h"
#include "ui_exposewidget.h"
#include <CommunicatorSingleton.h>
#include <imagedisplaywidget.h>
#include <IceConversions.h>
#include <QFileDialog>
#include <AstroIO.h>
#include <QMessageBox>
#include <sys/stat.h>
#include "downloadthread.h"
#include "exposedownloaddialog.h"
#include <ImageForwarder.h>

namespace snowgui {

/**
 * \brief Create a new exposewidget
 */
exposewidget::exposewidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::exposewidget) {
	ui->setupUi(this);

	_imageid = -1;
	_imageitem = NULL;
	_selectedfiles = 0;

	// add purposes to purpose combobox
	ui->purposeBox->addItem(QString("light"));
	ui->purposeBox->addItem(QString("dark"));
	ui->purposeBox->addItem(QString("flat"));
	ui->purposeBox->addItem(QString("bias"));
	ui->purposeBox->addItem(QString("test"));

	// create the columns
	QStringList	headers;
	headers << "No";
	headers << "Date";
	headers << "Time";
	headers << "Exposure";
	headers << "Temperature";
	headers << "Binning";
	headers << "Size";
	headers << "Filter";
	headers << "Bayer";
	ui->repositoryTree->setHeaderLabels(headers);
	ui->repositoryTree->header()->resizeSection(0, 80);
	ui->repositoryTree->header()->resizeSection(1, 100);
	ui->repositoryTree->header()->resizeSection(2, 80);
	ui->repositoryTree->header()->resizeSection(3, 60);
	ui->repositoryTree->header()->resizeSection(4, 80);
	ui->repositoryTree->header()->resizeSection(5, 50);
	ui->repositoryTree->header()->resizeSection(6, 100);
	ui->repositoryTree->header()->resizeSection(7, 60);

	// connections
	connect(ui->repositoryBox, SIGNAL(currentIndexChanged(QString)),
		this, SLOT(repositoryChanged(QString)));
	connect(ui->projectBox, SIGNAL(activated(QString)),
		this, SLOT(projectChanged(QString)));

	connect(ui->startButton, SIGNAL(clicked()),
		this, SLOT(startClicked()));

	connect(ui->saveButton, SIGNAL(clicked()),
		this, SLOT(saveClicked()));
	connect(ui->openButton, SIGNAL(clicked()),
		this, SLOT(openClicked()));
	connect(ui->deleteButton, SIGNAL(clicked()),
		this, SLOT(deleteClicked()));
	connect(ui->downloadButton, SIGNAL(clicked()),
		this, SLOT(downloadClicked()));

	connect(ui->repositoryTree,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		this,
		SLOT(currentImageChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(ui->repositoryTree,
		SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));
}

/**
 * \brief Destroy the expose widget
 */
exposewidget::~exposewidget() {
	delete ui;
}

/**
 * \brief Setup of the instrument widget fields
 */
void	exposewidget::instrumentSetup(astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// connect to the repository service
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
        Ice::ObjectPrx  base = ic->stringToProxy(
                        serviceobject.connect("Repositories"));
        snowstar::RepositoriesPrx      repositories
                = snowstar::RepositoriesPrx::checkedCast(base);
        if (!base) {
                throw std::runtime_error("cannot create repository app");
        }
        setRepositories(repositories);
}

/**
 * \brief Set the repositories proxy and read the repository names
 */
void	exposewidget::setRepositories(snowstar::RepositoriesPrx repositories) {
	_repositories = repositories;
	while (ui->repositoryBox->count() > 0) {
		ui->repositoryBox->removeItem(0);
	}
	ui->repositoryBox->setEnabled(true);
	_repository = NULL;
	if (!_repositories) {
		return;
	}
	snowstar::reponamelist	list = _repositories->list();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d repository names", list.size());
	if (0 == list.size()) {
		QMessageBox     message(this);
		message.setText(QString("No repositories"));
		message.setInformativeText(QString("No repositories were found. Exposed images cannot be saved."));
		message.exec();
		return;
	}
	QComboBox	*box = ui->repositoryBox;
	ui->repositoryBox->blockSignals(true);
	for (auto ptr = list.begin(); ptr != list.end(); ptr++) {
		std::string	reponame = *ptr;
		box->addItem(QString(reponame.c_str()));
		if (!_repository) {
			_repositoryname = reponame;
			_repository = _repositories->get(_repositoryname);
		}
		ui->repositoryBox->setCurrentIndex(0);
	}
	ui->repositoryBox->setEnabled(true);
	ui->repositoryBox->blockSignals(false);
	repositoryChanged(QString(_repositoryname.c_str()));

	// update the file list
	updateImagelist();
}

/**
 * \brief Slot to handle a change of repository
 */
void	exposewidget::repositoryChanged(const QString& repositoryname) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repository changed: %s",
		repositoryname.toLatin1().data());
	_repositoryname = std::string(repositoryname.toLatin1().data());
	_repository = _repositories->get(_repositoryname);

	// now that we have a repository, lets retrieve the project name
	// strings
	QString	currentproject = ui->projectBox->currentText();
	ui->projectBox->blockSignals(true);
	while (ui->projectBox->count() > 0) {
		ui->projectBox->removeItem(0);
	}
	int	currentindex = -1;
	if (_repository) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add project names");
		snowstar::projectnamelist projects
			= _repository->getProjectnames();
		for (auto ptr = projects.begin(); ptr != projects.end(); ptr++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "project name: %s",
				ptr->c_str());
			QString	project(ptr->c_str());
			if (project == currentproject) {
				currentindex = ui->projectBox->count();
			}
			ui->projectBox->addItem(project);
		}
	}
	ui->projectBox->blockSignals(false);
	if (currentindex >= 0) {
		ui->projectBox->setCurrentIndex(currentindex);
	} else {
		ui->projectBox->setEditText(currentproject);
	}

	updateImagelist();
}

/**
 * \brief Slot used to start focusing
 */
void	exposewidget::startClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start clicked");
	if (ui->startButton->text() == QString("Start")) {
		emit startExposure();
		ui->startButton->setText(QString("Cancel"));
		ui->exposuresLabel->setText(QString("Remaining:"));
		ui->exposuresSpinBox->setEnabled(false);
		return;
	}
	if (ui->startButton->text() == QString("Cancel")) {
		ui->exposuresSpinBox->setValue(1);
		ui->startButton->setText(QString("Start"));
		ui->exposuresLabel->setText(QString("Exposures:"));
		ui->exposuresSpinBox->setEnabled(true);
		return;
	}
}

/**
 * \brief Slot to handle a change of Project
 */
void	exposewidget::projectChanged(const QString& project) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "project changed");
	_projectname = std::string(project.toLatin1().data());
	updateImagelist();
}

/**
 * \brief Slot called when the filter wheel selection changed
 */
void	exposewidget::filterwheelSelected(snowstar::FilterWheelPrx filterwheel) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel changed");
	_filterwheel = filterwheel;
	updateHeaderlist();
}

/**
 * \brief Slot called when an image should be saved
 */
void	exposewidget::saveClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Save button clicked");
	if (_repositoryname.size() == 0) {
		return;
	}
	ImagePtr        imageptr = currentImage();
	QFileDialog     filedialog(this);
	filedialog.setAcceptMode(QFileDialog::AcceptSave);
	filedialog.setFileMode(QFileDialog::AnyFile);
	filedialog.setDefaultSuffix(QString("fits"));
	if (filedialog.exec()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "file dialog returned");
		QStringList     list = filedialog.selectedFiles();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "list size: %d", list.size());
		if (list.size() == 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "nothing selected");
			QMessageBox     message(&filedialog);
			message.setText(QString("No filename"));
			std::ostringstream      o;
			o << "The image file could not be saved because ";
			o << "no file name was selected";
			message.setInformativeText(QString(o.str().c_str()));
			message.exec();
			return;
		}
		std::string     filename(list.begin()->toLatin1().data());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s",
			filename.c_str());
		astro::io::FITSout      out(filename);
		if (out.exists()) {
			out.unlink();
		}
		try {
			out.write(imageptr);
		} catch (astro::io::FITSexception& x) {
			// find out whether file already exists
			QMessageBox     message(&filedialog);
			message.setText(QString("Save failed"));
			std::ostringstream      o;
			o << "Saving image to file '" << filename;
			o << "' failed. Cause: " << x.what();
			message.setInformativeText(QString(o.str().c_str()));
			message.exec();
		}
	}
}

/**
 * \brief auxiliary function to get the current Image 
 */
astro::image::ImagePtr	exposewidget::currentImage() {
	if (_imageid < 0) {
		return	ImagePtr();
	}
	if (!_repository) {
		return ImagePtr();
	}
	try {
		snowstar::ImageFile	imagefile = _repository->getImage(_imageid);
		return snowstar::convertfile(imagefile);
	} catch (const std::exception& x) {
	}
	return	ImagePtr();
}

/**
 * \brief Slot called when an image is to be opened
 */
void	exposewidget::openClicked() {
        ImagePtr        imageptr = currentImage();
	if (!imageptr) {
		return;
	}
        imagedisplaywidget      *idw = new imagedisplaywidget(NULL);
        connect(idw, SIGNAL(rectangleSelected(astro::image::ImageRectangle)),
                idw, SLOT(selectRectangle(QRect)));
	connect(idw,
		SIGNAL(offerImage(astro::image::ImagePtr, std::string)),
		ImageForwarder::get(),
		SLOT(sendImage(astro::image::ImagePtr, std::string)));
        idw->setRectangleSelectionEnabled(true);
        idw->setImage(imageptr);
	std::string	title = astro::stringprintf("Image %d from Repository %s",
		_imageid, _repositoryname.c_str());
	idw->setWindowTitle(QString(title.c_str()));
        idw->show();
}

/**
 * \brief Delete a file in the repository
 */
void	exposewidget::deleteClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Delete button clicked");
	if (_imageid < 0) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete image %d", _imageid);
	_repository->remove(_imageid);
	QTreeWidgetItem	*top = _imageitem->parent();
	for (int index = 0; index < top->childCount(); index++) {
		if (_imageid == top->child(index)->text(0).toInt()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying the entry");
			delete top->takeChild(index);
		}
	}
}

/**
 * \brief update the list of files
 */
void	exposewidget::updateHeaderlist() {
	// clean out the list
	_repository_sections.clear();
	_repository_index.clear();
	while (ui->repositoryTree->topLevelItemCount() > 0) {
		delete ui->repositoryTree->takeTopLevelItem(0);
	}

	// create the set of sections
	int	index = 0;

	{
		RepositoryKey	key(snowstar::ExLIGHT);
		_repository_index.insert(std::make_pair(key, index));
		RepositorySection	section(key, index++);
		_repository_sections.push_back(section);
	}

	if (_filterwheel) {
		int	n = _filterwheel->nFilters();
		for (int pos = 0; pos < n; pos++) {
			std::string	filtername
				= astro::trim(_filterwheel->filterName(pos));
			RepositoryKey	key(snowstar::ExLIGHT, filtername);
			_repository_index.insert(std::make_pair(key, index));
			RepositorySection	section(key, pos, index++);
			_repository_sections.push_back(section);
		}
	}
	
	{
		RepositoryKey	key(snowstar::ExDARK);
		_repository_index.insert(std::make_pair(key, index));
		RepositorySection	section(key, index++);
		_repository_sections.push_back(section);
	}
		
	{
		RepositoryKey	key(snowstar::ExFLAT);
		_repository_index.insert(std::make_pair(key, index));
		RepositorySection	section(key, index++);
		_repository_sections.push_back(section);
	}

	if (_filterwheel) {
		int	n = _filterwheel->nFilters();
		for (int pos = 0; pos < n; pos++) {
			std::string	filtername
				= astro::trim(_filterwheel->filterName(pos));
			RepositoryKey	key(snowstar::ExFLAT, filtername);
			_repository_index.insert(std::make_pair(key, index));
			RepositorySection	section(key, pos, index++);
			_repository_sections.push_back(section);
		}
	}

	{
		RepositoryKey	key(snowstar::ExBIAS);
		_repository_index.insert(std::make_pair(key, index));
		RepositorySection	section(key, index++);
		_repository_sections.push_back(section);
	}
	
	{
		RepositoryKey	key(snowstar::ExTEST);
		_repository_index.insert(std::make_pair(key, index));
		RepositorySection	section(key, index++);
		_repository_sections.push_back(section);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d sections",
		_repository_sections.size());

	// create the top level items
	for (auto ptr = _repository_sections.begin();
		ptr != _repository_sections.end(); ptr++) {
		QStringList     list;
		list << QString(ptr->purposeString().c_str());
		list << QString(ptr->filtername().c_str());
		QTreeWidgetItem *item = new QTreeWidgetItem(list,
			QTreeWidgetItem::Type);
		ui->repositoryTree->addTopLevelItem(item);
	}
	updateImagelist();
}

/**
 * \brief Create the list
 */
void	exposewidget::updateImagelist() {
	if (!_repository) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no repository");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "updating the image list");

	// remove all children
	int	n = ui->repositoryTree->topLevelItemCount();
	for (int i = 0; i < n; i++) {
		QTreeWidgetItem	*top = ui->repositoryTree->topLevelItem(i);
		while (top->childCount() > 0) {
			delete top->takeChild(0);
		}
	}

	std::string	condition = astro::stringprintf("project = '%s'",
				_projectname.c_str());
	snowstar::idlist	ids = _repository->getIdsCondition(condition);
	_selectedfiles = ids.size();
	ui->downloadButton->setEnabled(_selectedfiles > 0);
	for (auto ptr = ids.begin(); ptr != ids.end(); ptr++) {
		int	id = *ptr;
		snowstar::ImageInfo	info = _repository->getInfo(id);

		QStringList	list;
		list << QString::number(id);	// 0

		time_t	now;
		time(&now);
		now -= (int)info.observationago;
		char	buffer[100];
		struct tm	*tmp = localtime(&now);
		strftime(buffer, sizeof(buffer), "%F", tmp);
		list << QString(buffer);	// 1

		strftime(buffer, sizeof(buffer), "%T", tmp);
		list << QString(buffer);	// 2

		list << QString(astro::stringprintf("%.3f",
			info.exposuretime).c_str());	// 3

		list << QString(astro::stringprintf("%.1f",
			info.temperature).c_str());	// 4

		list << QString(astro::stringprintf("%d x %d", info.binning.x,
			info.binning.y).c_str());	// 5

		list << QString(astro::stringprintf("%d x %d", info.size.width,
			info.size.height).c_str());	// 6

		list << QString(info.filter.c_str());	// 7

		list << QString(info.bayer.c_str());	// 8

		QTreeWidgetItem	*item = new QTreeWidgetItem(list);
		item->setTextAlignment(0, Qt::AlignRight);
		item->setTextAlignment(3, Qt::AlignRight);
		item->setTextAlignment(4, Qt::AlignRight);
		item->setTextAlignment(5, Qt::AlignCenter);

		RepositoryKey	key;
		if (((info.purpose == "light") || (info.purpose == "flat")) && (info.filter.size() > 0)) {
			key = RepositoryKey(info.purpose, info.filter);
		} else {
			key = RepositoryKey(info.purpose);
		}
		auto s = _repository_index.find(key);
		if (s == _repository_index.end()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "key not found: '%s', try again without filter",
				key.toString().c_str());
			key = RepositoryKey(info.purpose);
			s = _repository_index.find(key);
		}
		if (s != _repository_index.end()) {
			QTreeWidgetItem	*top
				= ui->repositoryTree->topLevelItem(s->second);
			top->addChild(item);
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "key not found: '%s'",
				key.toString().c_str());
		}
	}
}

/**
 * \brief Slot called when the current item changes
 *
 * This method retrieves the current repository name and the current
 * image id. The currentImage() method retrieves the image indicated by
 * these two members from the repository. They are also used by the
 * button slots to perform actions on an image.
 */
void    exposewidget::currentImageChanged(QTreeWidgetItem *current,
	QTreeWidgetItem *previous) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "currentItemChanged(%p, %p)", current, previous);
	if (NULL == current) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no current item");
		return;
	}
	if (NULL == current->parent()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "has not parent");
		_imageid = -1;
		_imageitem = NULL;
		ui->saveButton->setEnabled(false);
		ui->openButton->setEnabled(false);
		ui->deleteButton->setEnabled(false);
		return;
	}
	// first find out whether this is a top level item
	if (ui->repositoryTree->invisibleRootItem() == current->parent()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "top level item");
		_imageid = -1;
		_imageitem = NULL;
		ui->saveButton->setEnabled(false);
		ui->openButton->setEnabled(false);
		ui->deleteButton->setEnabled(false);
		return;
	}
	_imageid = current->text(0).toInt();
	_imageitem = current;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current: image = %d", _imageid);
	ui->saveButton->setEnabled(true);
	ui->openButton->setEnabled(true);
	ui->deleteButton->setEnabled(true);
}

/**
 * \brief Slot called when an item is double clicked
 *
 * This opens the image just as if the open button was clicked.
 */
void    exposewidget::itemDoubleClicked(QTreeWidgetItem *, int) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "itemDoubleClicked()");
	openClicked();
}

/**
 * \brief Process an image reveived as an image proxy
 */
void	exposewidget::imageproxyReceived(snowstar::ImagePrx imageproxy) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new image proxy received");
	// add additional fields
	snowstar::Metadata	metadata;
	if (_filterwheel) {
		snowstar::Metavalue	v;
		v.keyword = "FILTER";
		v.value = astro::trim(_filterwheel->filterName(_filterwheel->currentPosition()));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filter = '%s'", v.value.c_str());
		metadata.push_back(v);
	}

	{
		snowstar::Metavalue	v;
		v.keyword = "PROJECT";
		v.value = std::string(ui->projectBox->currentText().toLatin1().data());
		metadata.push_back(v);
	}

	{
		snowstar::Metavalue	v;
		v.keyword = "PURPOSE";
		v.value = std::string(ui->purposeBox->currentText().toLatin1().data());
		metadata.push_back(v);
	}
	imageproxy->setMetadata(metadata);

	if (_repository) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "moveing the image to repo %s",
			_repositoryname.c_str());
		imageproxy->toRepository(_repositoryname);
		imageproxy->remove();
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "updating the image list");
	projectChanged(ui->projectBox->currentText());

	// decrement the value in the 
	int	count = ui->exposuresSpinBox->value();
	if (count == 1) {
		ui->startButton->setText(QString("Start"));
		ui->exposuresLabel->setText(QString("Exposures:"));
		ui->exposuresSpinBox->setEnabled(true);
		return;
	}
	ui->exposuresSpinBox->setValue(count - 1);
	emit startExposure();
}

/**
 * \brief Download the whole project
 */
void	exposewidget::downloadClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "download clicked");
	if (_selectedfiles == 0) {
		return;
	}

	QFileDialog	filedialog(this);
	filedialog.setAcceptMode(QFileDialog::AcceptOpen);
	filedialog.setFileMode(QFileDialog::DirectoryOnly);
	if (!filedialog.exec()) {
		return;
	}

	QStringList	list = filedialog.selectedFiles();
	std::string	dirname(list.begin()->toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "directory: %s",
		dirname.c_str());

	downloadlist	filelist;

	int	n = ui->repositoryTree->topLevelItemCount();
	for (int index = 0; index < n; index++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "section %d", index);
		std::string	purpose
			= _repository_sections[index].purposeString();
		std::string	dir = dirname + "/" + purpose;
		std::string	filter
			= _repository_sections[index].filtername();
		std::string	dir2 = dir + "/" + filter;
		QTreeWidgetItem	*top
			= ui->repositoryTree->topLevelItem(index);
		if (top->childCount() == 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no children");
			continue;
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d children",
				top->childCount());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "mkdir(%s)",
			dir.c_str());
		mkdir(dir.c_str(), 0777);
		if (filter.size() > 0) {
			dir = dir2;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "mkdir(%s)",
				dir.c_str());
			mkdir(dir.c_str(), 0777);
		}
		for (int i = 0; i < top->childCount(); i++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "adding child %d", i);
			int	imageid = top->child(i)->text(0).toInt();
			std::string	filename
				= astro::stringprintf("%s-%05d.fits",
					_repositoryname.c_str(), imageid);
			downloaditem	item(_repositoryname, imageid,
						dir, filename);
			filelist.push_back(item);
		}
	}

	// start the download dialog
	exposedownloaddialog	*edd = new exposedownloaddialog(this);
	edd->set(_repositories, filelist);
	edd->exec();
}

} // namespace snowgui
