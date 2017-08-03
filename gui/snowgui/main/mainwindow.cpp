/*
 * mainwindow.cpp -- implementation of the main window
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include "serverselectiondialog.h"
#include "instrumentselectiondialog.h"
#include <takeimagewindow.h>
#include <focusingwindow.h>
#include <guidingwindow.h>
#include <instrumentswindow.h>
#include "configurationdialog.h"
#include <imageswindow.h>
#include <repositorywindow.h>
#include <taskwindow.h>
#include <QMessageBox>
#include <QFileDialog>
#include <sstream>
#include <exposewindow.h>
#include <AstroIO.h>
#include <imagedisplaywidget.h>
#include <browserwindow.h>

using namespace astro::discover;

namespace snowgui {

/**
 * \brief Create a MainWindow Widget
 */
MainWindow::MainWindow(QWidget *parent,
	const astro::discover::ServiceObject serviceobject)
	: QMainWindow(parent), _serviceobject(serviceobject),
	  ui(new Ui::MainWindow) {
	// create user interface components
	ui->setupUi(this);

	// connect buttons
	connect(ui->appPreviewButton, SIGNAL(clicked()),
		this, SLOT(launchPreview()));
	connect(ui->appFocusingButton, SIGNAL(clicked()),
		this, SLOT(launchFocusing()));
	connect(ui->appGuidingButton, SIGNAL(clicked()),
		this, SLOT(launchGuiding()));
	connect(ui->appInstrumentsButton, SIGNAL(clicked()),
		this, SLOT(launchInstruments()));
	connect(ui->appRepositoryButton, SIGNAL(clicked()),
		this, SLOT(launchRepository()));
	connect(ui->appTasksButton, SIGNAL(clicked()),
		this, SLOT(launchTasks()));
	connect(ui->appConfigurationButton, SIGNAL(clicked()),
		this, SLOT(launchConfiguration()));
	connect(ui->appImagesButton, SIGNAL(clicked()),
		this, SLOT(launchImages()));
	connect(ui->appExposeButton, SIGNAL(clicked()),
		this, SLOT(launchExpose()));

	// initialize application specific stuff
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting main window with server %s",
		_serviceobject.toString().c_str());
	setWindowTitle(QString(_serviceobject.toString().c_str()));

	// find out which services are actually offered on that server
	setServiceLabelEnabled(ServiceSubset::INSTRUMENTS);
	setServiceLabelEnabled(ServiceSubset::TASKS);
	setServiceLabelEnabled(ServiceSubset::DEVICES);
	setServiceLabelEnabled(ServiceSubset::GUIDING);
	setServiceLabelEnabled(ServiceSubset::FOCUSING);
	setServiceLabelEnabled(ServiceSubset::IMAGES);
	setServiceLabelEnabled(ServiceSubset::REPOSITORY);

	// decide which services to enable
	if (_serviceobject.has(ServiceSubset::INSTRUMENTS)) {
		ui->appInstrumentsButton->setEnabled(true);
		if (_serviceobject.has(ServiceSubset::DEVICES)) {
			ui->appPreviewButton->setEnabled(true);
			ui->appFocusingButton->setEnabled(true);
			ui->appExposeButton->setEnabled(true);
			if (_serviceobject.has(ServiceSubset::GUIDING)) {
				ui->appGuidingButton->setEnabled(true);
			}
		}
	}
	if (_serviceobject.has(ServiceSubset::REPOSITORY)) {
		ui->appRepositoryButton->setEnabled(true);
	}
	if (_serviceobject.has(ServiceSubset::TASKS)) {
		ui->appTasksButton->setEnabled(true);
	}
	if (_serviceobject.has(ServiceSubset::IMAGES)) {
		ui->appImagesButton->setEnabled(true);
	}

	// add menu
	createActions();
	createMenus();
}

/**
 * \brief Destroy the MainWindow widget
 */
MainWindow::~MainWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy MainWindow");
	delete ui;
}

/**
 * \brief Launch the Preview subapplication
 */
void	MainWindow::launchPreview() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch a preview subapplication");
	try {
		InstrumentSelectionApplication<snowgui::takeimagewindow>	*is
			= new InstrumentSelectionApplication<snowgui::takeimagewindow>(this, _serviceobject);
		is->setWindowTitle(QString("Select instrument for Preview "
			"application"));
		is->exec();
		delete is;
	} catch (const std::exception& x) {
		QMessageBox	message;
		message.setText(QString("Cannot launch Preview"));
		message.setInformativeText(QString(astro::stringprintf(
			"The Preview subapplication could not be started. "
			"Cause: %s", x.what()).c_str()));
		message.exec();
	}
}

/**
 * \brief Launch the Focusing subapplication
 */
void	MainWindow::launchFocusing() {
	try {
		InstrumentSelectionApplication<snowgui::focusingwindow>	*is
			= new InstrumentSelectionApplication<snowgui::focusingwindow>(this, _serviceobject);
		is->setWindowTitle(QString("Select instrument for Focusing "
			"application"));
		is->exec();
		delete is;
	} catch (const std::exception& x) {
		QMessageBox	message;
		message.setText(QString("Cannot launch Focusing"));
		message.setInformativeText(QString(astro::stringprintf(
			"The Focusing subapplication could not be started. "
			"Cause: %s", x.what()).c_str()));
		message.exec();
	}
}

/**
 * \brief Launch the Guiging subapplication
 */
void	MainWindow::launchGuiding() {
	try {
		InstrumentSelectionApplication<snowgui::guidingwindow>	*is
			= new InstrumentSelectionApplication<snowgui::guidingwindow>(this, _serviceobject);
		is->setWindowTitle(QString("Select instrument for Guiding "
			"application"));
		is->exec();
		delete is;
	} catch (const std::exception& x) {
		QMessageBox	message;
		message.setText(QString("Cannot launch Guiding"));
		message.setInformativeText(QString(astro::stringprintf(
			"The Guiding subapplication could not be started. "
			"Cause: %s", x.what()).c_str()));
		message.exec();
	}
}

/**
 * \brief Launch the instruments application
 */
void	MainWindow::launchInstruments() {
	try {
		instrumentswindow	*iw = new instrumentswindow(NULL,
			_serviceobject);
		iw->show();
	} catch (const std::exception& x) {
		QMessageBox	*message = new QMessageBox(this);
		message->setText(QString("Connection failure"));
		std::ostringstream	out;
		out << "Failed to connect to the 'Instruments' service on '";
		out << _serviceobject.toString();
		out << "'. Instruments Window cannot be constructed. ";
		out << "Cause: ";
		out << x.what();
		message->setInformativeText(QString(out.str().c_str()));
		message->exec();
		delete message;
	}
}

/**
 * \brief Launch the Configuration application
 */
void	MainWindow::launchConfiguration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch configuration window");
	try {
		configurationdialog	*config = new configurationdialog(NULL,
			_serviceobject);
		config->show();
	} catch (const std::exception& x) {
		QMessageBox	*message = new QMessageBox(this);
		message->setText(QString("Connection failure"));
		std::ostringstream	out;
		out << "Failed to connect to the 'Configuration' service on ";
		out << _serviceobject.toString();
		out << ". Configuration dialog cannot be constructed.";
		out << "Cause: ";
		out << x.what();
		message->setInformativeText(QString(out.str().c_str()));
		message->exec();
		delete message;
	}
}

/**
 * \brief Launch the Images subapplication
 */
void	MainWindow::launchImages() {
	try {
		imageswindow	*images = new imageswindow(NULL,
			_serviceobject);
		images->show();
	} catch (const std::exception& x) {
		QMessageBox	*message = new QMessageBox(this);
		message->setText(QString("Connection failure"));
		std::ostringstream	out;
		out << "Failed to connect to the 'Images' service on ";
		out << _serviceobject.toString();
		out << ". Images window cannot be constructed.";
		out << "Cause: ";
		out << x.what();
		message->setInformativeText(QString(out.str().c_str()));
		message->exec();
		delete message;
	}
}

/**
 * \brief Launch the Repository managment subapplication
 */
void	MainWindow::launchRepository() {
	try {
		repositorywindow	*rw = new repositorywindow(NULL,
			_serviceobject);
		rw->show();
	} catch (const std::exception& x) {
		QMessageBox	*message = new QMessageBox(this);
		message->setText(QString("Connection failure"));
		std::ostringstream	out;
		out << "Failed to connect to the 'Repository' service on ";
		out << _serviceobject.toString();
		out << ". Repository window cannot be constructed.";
		out << "Cause: ";
		out << x.what();
		message->setInformativeText(QString(out.str().c_str()));
		message->exec();
		delete message;
	}
}

/**
 * \brief Launch the Tasks subapplication
 */
void	MainWindow::launchTasks() {
	try {
		InstrumentSelectionApplication<snowgui::taskwindow>	*is
			= new InstrumentSelectionApplication<snowgui::taskwindow>(this, _serviceobject);
		is->setWindowTitle(QString("Select instrument for Task application"));
		is->exec();
		delete is;
	} catch (const std::exception& x) {
		QMessageBox	message(this);
		message.setText(QString("Connection failure"));
		std::ostringstream	out;
		out << "Failed to connect to the 'Repository' service on ";
		out << _serviceobject.toString();
		out << ". Task window cannot be constructed.";
		out << "Cause: ";
		out << x.what();
		message.setInformativeText(QString(out.str().c_str()));
		message.exec();
	}
}

/**
 * \brief Launch the Expose subapplication
 */
void	MainWindow::launchExpose() {
	try {
		InstrumentSelectionApplication<snowgui::exposewindow>	*is
			= new InstrumentSelectionApplication<snowgui::exposewindow>(this, _serviceobject);
		is->setWindowTitle(QString("Select instrument for Expose application"));
		is->exec();
		delete is;
	} catch (const std::exception& x) {
		QMessageBox	message;
		message.setText(QString("Cannot launch Expose"));
		message.setInformativeText(QString(astro::stringprintf(
			"The Expose subapplication could not be started. "
			"Cause: %s", x.what()).c_str()));
		message.exec();
	}
}

/**
 * \brief Connect action from the file menu
 */
void	MainWindow::connectFile() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connect action invoked");

	ServiceDiscoveryPtr	servicediscovery = ServiceDiscovery::get();
	sleep(1);
	ServerSelectionDialog	*sd = new ServerSelectionDialog(this,
		servicediscovery);
	sd->show();
}

/**
 * \brief Open a FITS file and display it
 */
void	MainWindow::openFile() {
	QFileDialog     filedialog(this);
	filedialog.setAcceptMode(QFileDialog::AcceptOpen);
	filedialog.setFileMode(QFileDialog::AnyFile);
	filedialog.setDefaultSuffix(QString("fits"));
	if (!filedialog.exec()) {
		return;
	}
	QStringList	list = filedialog.selectedFiles();
	for (auto ptr = list.begin(); ptr != list.end(); ptr++) {
		// open file
		std::string	filename(ptr->toLatin1().data());
		astro::Path	p(filename);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "open file: %s",
			filename.c_str());
		astro::io::FITSin	in(filename);
		try {
			ImagePtr	image = in.read();
			imagedisplaywidget	*idw = new imagedisplaywidget(NULL);
			idw->setImage(image);
			idw->setWindowTitle(QString(p.basename().c_str()));
			idw->show();
		} catch (const std::exception& x) {
			// failure 
			debug(LOG_ERR, DEBUG_LOG, 0, "%s: %s", filename.c_str(), x.what());
		}
	}
}

/**
 * \brief Open a directory in the browser
 */
void	MainWindow::browseDirectory() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open browser");
	QFileDialog     filedialog(this);
	filedialog.setAcceptMode(QFileDialog::AcceptOpen);
	filedialog.setFileMode(QFileDialog::DirectoryOnly);
	if (!filedialog.exec()) {
		return;
	}
	QStringList     list = filedialog.selectedFiles();
	std::string     dirname(list.begin()->toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "directory: %s",
		dirname.c_str());
	browserwindow	*browser = new browserwindow(NULL);
	browser->setDirectory(dirname);
	browser->show();
}

/**
 * \brief Save the image currently offered for saving
 */
void	MainWindow::saveImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "save an image");
	if (!_image) {
		return;
	}

	QFileDialog     filedialog(this);
	filedialog.setAcceptMode(QFileDialog::AcceptSave);
	filedialog.setFileMode(QFileDialog::AnyFile);
        filedialog.setDefaultSuffix(QString("fits"));
	if (!filedialog.exec()) {
		return;
	}
	QStringList     list = filedialog.selectedFiles();
	std::string     filename(list.begin()->toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file: %s",
		filename.c_str());

	astro::io::FITSout	out(filename);
	out.write(_image);
}

/**
 * \brief Create the actions in the menu
 */
void	MainWindow::createActions() {
	connectAction = new QAction(QString("Connect"), this);
	connect(connectAction, &QAction::triggered, this,
		&MainWindow::connectFile);

	openAction = new QAction(QString("Open"), this);
	connect(openAction, &QAction::triggered, this,
		&MainWindow::openFile);

	browseAction = new QAction(QString("Browse"), this);
	connect(browseAction, &QAction::triggered, this,
		&MainWindow::browseDirectory);

	saveAction = new QAction(QString("Save"), this);
	saveAction->setEnabled(false);
	connect(saveAction, &QAction::triggered, this,
		&MainWindow::saveImage);
}

/**
 * \brief Create the menus of the main window
 */
void	MainWindow::createMenus() {
	fileMenu = menuBar()->addMenu(QString("File"));
	fileMenu->addAction(connectAction);
	fileMenu->addAction(openAction);
	fileMenu->addAction(browseAction);
	fileMenu->addAction(saveAction);
}

/**
 * \brief Set the attributes of the service label
 */
void	MainWindow::setServiceLabelEnabled(ServiceSubset::service_type t) {
	QLabel	*l = serviceLabel(t);
	if (_serviceobject.has(t)) {
		l->setStyleSheet("QLabel { background-color : white; color : black; }");
	} else {
		l->setStyleSheet("QLabel { background-color : transparent; color : grey; }");
	}
}

/**
 * \brief Get the label for a given service
 */
QLabel	*MainWindow::serviceLabel(ServiceSubset::service_type t) {
	switch (t) {
	case ServiceSubset::INSTRUMENTS:
		return ui->instrumentsLabel;
	case ServiceSubset::TASKS:
		return ui->tasksLabel;
	case ServiceSubset::DEVICES:
		return ui->devicesLabel;
	case ServiceSubset::GUIDING:
		return ui->guidingLabel;
	case ServiceSubset::FOCUSING:
		return ui->focusingLabel;
	case ServiceSubset::IMAGES:
		return ui->imagesLabel;
	case ServiceSubset::REPOSITORY:
		return ui->repositoryLabel;
	}
	return NULL;
}

void	MainWindow::imageForSaving(astro::image::ImagePtr image) {
	_image = image;
	if (_image) {
		saveAction->setEnabled(true);
	} else {
		saveAction->setEnabled(false);
	}
}

} // namespace snowgui
