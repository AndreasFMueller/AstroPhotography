/*
 * repositorywindow.cpp -- 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "repositorywindow.h"
#include "ui_repositorywindow.h"
#include <CommunicatorSingleton.h>
#include <QFileDialog>
#include <QMessageBox>
#include <AstroIO.h>
#include <IceConversions.h>
#include <imagedisplaywidget.h>
#include "repositorysavedialog.h"

namespace snowgui {

/**
 * \brief Construct a new repository window
 */
repositorywindow::repositorywindow(QWidget *parent,
	astro::discover::ServiceObject serviceobject)
	: QWidget(parent), ui(new Ui::repositorywindow),
	  _serviceobject(serviceobject) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing repository window");
	ui->setupUi(this);

	// make sure current item is properly initialized
	_reponame = std::string("");
	_imageid = -1;

	// add the headers to the tree view
	QStringList	headers;
	headers << "No";		// 0
	headers << "Project";		// 1
	headers << "Purpose";		// 2
	headers << "Date";		// 3
	headers << "Time";		// 4
	headers << "Exposure";		// 5
	headers << "Temperature";	// 6
	headers << "Binning";		// 7
	headers << "Size";		// 8
	headers << "Filter";		// 9
	headers << "Bayer";		// 10
	headers << "Focus";		// 11
	headers << "Filename";		// 12
	headers << "UUID";		// 13
	ui->repositoryTree->setHeaderLabels(headers);
	ui->repositoryTree->header()->resizeSection(0, 80);
	ui->repositoryTree->header()->resizeSection(1, 100);
	ui->repositoryTree->header()->resizeSection(2, 80);
	ui->repositoryTree->header()->resizeSection(3, 100);
	ui->repositoryTree->header()->resizeSection(4, 80);
	ui->repositoryTree->header()->resizeSection(5, 60);
	ui->repositoryTree->header()->resizeSection(6, 80);
	ui->repositoryTree->header()->resizeSection(7, 50);
	ui->repositoryTree->header()->resizeSection(8, 100);
	ui->repositoryTree->header()->resizeSection(9, 100);
	ui->repositoryTree->header()->resizeSection(10, 80);
	ui->repositoryTree->header()->resizeSection(11, 80);
	ui->repositoryTree->header()->resizeSection(12, 190);

	// window title
	std::string	title = astro::stringprintf("Repository overview on %s",
		_serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	// connect to the repository server
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
        Ice::ObjectPrx  base = ic->stringToProxy(
                        _serviceobject.connect("Repositories"));
        snowstar::RepositoriesPrx      repositories
                = snowstar::RepositoriesPrx::checkedCast(base);
        if (!base) {
                throw std::runtime_error("cannot create repository app");
        }
        setRepositories(repositories);

	// connections
	connect(ui->refreshButton, SIGNAL(clicked()),
		this, SLOT(refreshClicked()));
	connect(ui->saveButton, SIGNAL(clicked()),
		this, SLOT(saveClicked()));
	connect(ui->openButton, SIGNAL(clicked()),
		this, SLOT(openClicked()));
	connect(ui->previewButton, SIGNAL(clicked()),
		this, SLOT(previewClicked()));
	connect(ui->deleteButton, SIGNAL(clicked()),
		this, SLOT(deleteClicked()));
	connect(ui->repositoryTree,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		this,
		SLOT(currentImageChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(ui->repositoryTree,
		SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));
}

/**
 * \brief Destroy a repository window
 */
repositorywindow::~repositorywindow() {
	delete ui;
}

/**
 * \brief Accept a repository proxy
 *
 * This method gets all repositories on the remote servcer and downloads
 * all summary information for all images.
 */
void	repositorywindow::setRepositories(
		snowstar::RepositoriesPrx repositories) {
	_repositories = repositories;
	if (!_repositories) {
		return;
	}

	addAllImages();
}

/**
 * \brief Auxiliary function to add all images from a repo to a top level item
 */
void	repositorywindow::addImages(QTreeWidgetItem *top,
		const std::string& reponame) {
	snowstar::RepositoryPrx	repository = _repositories->get(reponame);
	snowstar::idlist	ids = repository->getIds();
	for (auto i = ids.begin(); i != ids.end(); i++) {
		int	id = *i;
		snowstar::ImageInfo	info = repository->getInfo(id);
		
		QStringList	list;

		list << QString::number(info.id);
		list << QString(info.project.c_str());
		list << QString(info.purpose.c_str());

		std::string	s;

		time_t	now;
		time(&now);
		now -= (int)info.observationago;
		struct tm	*tmp = localtime(&now);
		char	buffer[100];

		// date
		strftime(buffer, sizeof(buffer), "%F", tmp);
		list << QString(buffer);

		// time
		strftime(buffer, sizeof(buffer), "%T", tmp);
		list << QString(buffer);

		// exposure
		s = astro::stringprintf("%.3f", info.exposuretime);
		list << QString(s.c_str());

		// temperature
		s = astro::stringprintf("%.1f", info.temperature);
		list << QString(s.c_str());

		// binning
		s = astro::stringprintf("%d x %d", info.binning.x,
			info.binning.y);
		list << QString(s.c_str());

		// size
		s = astro::stringprintf("%d x %d", info.size.width,
			info.size.height);
		list << QString(s.c_str());

		// filter
		list << QString(info.filter.c_str());

		// bayer pattern
		list << QString(info.bayer.c_str());

		// focus position
		s = astro::stringprintf("%ld", info.focus);
		list << QString(s.c_str());

		// filename
		list << QString(info.filename.c_str());

		// UUID
		list << QString(info.uuid.c_str());

                QTreeWidgetItem *item = new QTreeWidgetItem(list,
                        QTreeWidgetItem::Type);
		item->setTextAlignment(0, Qt::AlignRight);
		item->setTextAlignment(5, Qt::AlignRight);
		item->setTextAlignment(6, Qt::AlignRight);
		item->setTextAlignment(7, Qt::AlignCenter);
		item->setTextAlignment(8, Qt::AlignCenter);
		item->setTextAlignment(11, Qt::AlignRight);
		
                top->addChild(item);
	}
}

/**
 * \brief
 */
void	repositorywindow::addAllImages() {
	if (!_repositories) {
		return;
	}
	ui->repositoryTree->blockSignals(true);
	// read a list of repository names
	snowstar::reponamelist	repos = _repositories->list();
	snowstar::reponamelist::const_iterator	i;
	for (i = repos.begin(); i != repos.end(); i++) {
		std::string	reponame = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "repository: %s",
			reponame.c_str());
		QStringList     list;
		list << QString("");
                list << QString(reponame.c_str());
                QTreeWidgetItem *item = new QTreeWidgetItem(list,
                        QTreeWidgetItem::Type);
                ui->repositoryTree->addTopLevelItem(item);
		addImages(item, reponame);
	}
	ui->repositoryTree->blockSignals(false);
}

/**
 * \brief What to do when the window closes
 */
void	repositorywindow::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

/**
 * \brief Auxiliary function to retrieve the current image from the repository
 */
astro::image::ImagePtr	repositorywindow::currentImage(snowstar::ImageEncoding encoding) {
	snowstar::RepositoryPrx	repository = _repositories->get(_reponame);
	snowstar::ImageBuffer	image = repository->getImage(_imageid, encoding);
	ImagePtr	imageptr = snowstar::convertimage(image);
	return imageptr;
}

/**
 * \brief Save currently selected images from the repository to file/directory
 */
void	repositorywindow::saveClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saveClicked()");
	if (_reponame.size() == 0) {
		return;
	}

	// find out how many images are selected
	QList<QTreeWidgetItem*>	selected = ui->repositoryTree->selectedItems();
	if (selected.count() > 1) {
		saveMulti(selected);
		return;
	}

	// save an individual image to a file
	ImagePtr	imageptr = currentImage(snowstar::ImageEncodingFITS);
	QFileDialog	filedialog(this);
	filedialog.setAcceptMode(QFileDialog::AcceptSave);
	filedialog.setFileMode(QFileDialog::AnyFile);
	filedialog.setDefaultSuffix(QString("fits"));
	if (filedialog.exec()) {
		QStringList	list = filedialog.selectedFiles();
		std::string	filename(list.begin()->toLatin1().data());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s");
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
 * \brief Save a set of images into a directory
 */
void	repositorywindow::saveMulti(QList<QTreeWidgetItem*>& items) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "save %d images", items.count());
	QString	dir = QFileDialog::getExistingDirectory(this,
		"Save images to directory", NULL,
		QFileDialog::ShowDirsOnly |
		QFileDialog::DontResolveSymlinks);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "directory: %s",
		dir.toLatin1().data());
	if (dir.size() == 0) {
		return;
	}
	// now we have all the information for the download. We extract
	// the repository names and ids from the selection 
	std::list<std::pair<std::string, int> >	imagelist;
	QList<QTreeWidgetItem*>::const_iterator	i;
	for (i = items.begin(); i != items.end(); i++) {
		if ((*i)->parent() == NULL) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "top level");
		} else {
			std::string	reponame
				= (*i)->parent()->text(1).toLatin1().data();
			int	imageid = (*i)->text(0).toInt();
			imagelist.push_back(std::make_pair(reponame, imageid));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "repo: %s, id %d",
				reponame.c_str(), imageid);
		}
	}

	// we have no prepared a list of 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saving %d images", imagelist.size());
	repositorysavedialog	d(this);
	d.set(std::string(dir.toLatin1().data()), _repositories, imagelist);
	d.exec();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "save dialog returned");
}

void	repositorywindow::showImage(astro::image::ImagePtr imageptr) {
	imagedisplaywidget	*idw = new imagedisplaywidget(NULL);
	connect(idw, SIGNAL(rectangleSelected(astro::image::ImageRectangle)),
        	idw, SLOT(selectRectangle(QRect)));
	idw->setRectangleSelectionEnabled(true);
	idw->setImage(imageptr);
	std::string	title
		= astro::stringprintf("Image %d from repository %s",
			_imageid, _reponame.c_str());
	idw->setWindowTitle(QString(title.c_str()));
	idw->show();
}

/**
 * \brief Open the current image from the repository
 */
void	repositorywindow::openClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "openClicked()");
	if (_reponame.size() == 0) {
		return;
	}
	ImagePtr	imageptr = currentImage(snowstar::ImageEncodingFITS);
	showImage(imageptr);
}

/**
 * \brief Open the current image from the repository
 */
void	repositorywindow::previewClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "openClicked()");
	if (_reponame.size() == 0) {
		return;
	}
	ImagePtr	imageptr = currentImage(snowstar::ImageEncodingJPEG);
	showImage(imageptr);
}


/**
 * \brief Delete multiple images
 */
void	repositorywindow::deleteMulti(QList<QTreeWidgetItem*>& items) {
	QMessageBox	message;
	message.setText(QString("Confirm delete"));
	std::ostringstream	out;
	out << "Do you really want to delete " << items.count();
	out << " images from repository " << _reponame;
	out << "?";
	message.addButton(QString("Cancel"), QMessageBox::RejectRole);
	message.addButton(QString("Delete"), QMessageBox::AcceptRole);
	message.setInformativeText(QString(out.str().c_str()));
	if (1 != message.exec()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "delete cancelled");
	}
	snowstar::RepositoryPrx	repository
		= _repositories->get(_reponame);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deleting image %d", _imageid);
	try {
		// iterate through the list of items
		QList<QTreeWidgetItem*>::iterator	li;
		for (li = items.begin(); li != items.end(); li++) {
			QTreeWidgetItem	*item = *li;

			// find the image id
			int	imageid = item->text(0).toInt();

			repository->remove(imageid);
			for (int i = 0; i < 13; i++) {
				ui->repositoryTree->removeItemWidget(item, i);
			}
			delete item;
		}
	} catch (...) { }
}

/**
 * \brief Slot called to delete the current image from the repository
 */
void	repositorywindow::deleteClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deleteClicked()");
	if (_reponame.size() == 0) {
		return;
	}
	QList<QTreeWidgetItem*>	selected = ui->repositoryTree->selectedItems();
	if (selected.count() > 1) {
		deleteMulti(selected);
		return;
	}
	QMessageBox	message;
	message.setText(QString("Confirm delete"));
	std::ostringstream	out;
	out << "Do you really want image " << _imageid;
	out << " from repository " << _reponame;
	out << "?";
	message.addButton(QString("Cancel"), QMessageBox::RejectRole);
	message.addButton(QString("Delete"), QMessageBox::AcceptRole);
	message.setInformativeText(QString(out.str().c_str()));
	if (1 == message.exec()) {
		snowstar::RepositoryPrx	repository
			= _repositories->get(_reponame);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "deleting image %d", _imageid);
		try {
			repository->remove(_imageid);
			QTreeWidgetItem	*item = ui->repositoryTree->currentItem();
			for (int i = 0; i < 13; i++) {
				ui->repositoryTree->removeItemWidget(item, i);
			}
			delete item;
		} catch (...) { }
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "delete cancelled");
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
void    repositorywindow::currentImageChanged(QTreeWidgetItem *current,
		QTreeWidgetItem * /* previous */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "currentItemChanged()");
	if (NULL == current) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no current item");
		return;
	}
	if (NULL == current->parent()) {
		return;
	}
	// first find out whether this is a top level item
	if (ui->repositoryTree->invisibleRootItem() == current->parent()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "top level item");
		_reponame = std::string("");
		_imageid = -1;
		ui->saveButton->setEnabled(false);
		ui->openButton->setEnabled(false);
		ui->deleteButton->setEnabled(false);
		return;
	}
	_reponame = std::string(current->parent()->text(1).toLatin1().data());
	_imageid = current->text(0).toInt();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current: repo = %s, image = %d",
		_reponame.c_str(), _imageid);
	ui->saveButton->setEnabled(true);
	ui->openButton->setEnabled(true);
	ui->deleteButton->setEnabled(true);
}

/**
 * \brief Slot called when an item is double clicked
 *
 * This opens the image just as if the open button was clicked.
 */
void    repositorywindow::itemDoubleClicked(QTreeWidgetItem *, int) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "itemDoubleClicked()");
	openClicked();
}

/**
 * \brief Slot called when we hit refresh
 */
void	repositorywindow::refreshClicked() {
	// remove all items in the tree
	while (ui->repositoryTree->topLevelItemCount()) {
		delete ui->repositoryTree->takeTopLevelItem(0);
	}

	// add all images
	addAllImages();
}

} // namespace snowgui
