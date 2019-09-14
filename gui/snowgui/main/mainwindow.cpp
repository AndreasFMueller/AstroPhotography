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
#include <pointingwindow.h>
#include <instrumentswindow.h>
#include "configurationdialog.h"
#include <imageswindow.h>
#include <repositorywindow.h>
#include <taskwindow.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QAction>
#include <sstream>
#include <exposewindow.h>
#include <AstroIO.h>
#include <imagedisplaywidget.h>
#include <browserwindow.h>
#include <ImageForwarder.h>
#include <eventdisplaywidget.h>
#include <CommunicatorSingleton.h>

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
	connect(ui->appPointingButton, SIGNAL(clicked()),
		this, SLOT(launchPointing()));
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
	connect(ui->appEventsButton, SIGNAL(clicked()),
		this, SLOT(launchEvents()));

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
	setServiceLabelEnabled(ServiceSubset::GATEWAY);

	// decide which services to enable
	if (_serviceobject.has(ServiceSubset::INSTRUMENTS)) {
		ui->appInstrumentsButton->setEnabled(true);
		if (_serviceobject.has(ServiceSubset::DEVICES)) {
			ui->appPreviewButton->setEnabled(true);
			ui->appFocusingButton->setEnabled(true);
			ui->appExposeButton->setEnabled(true);
			ui->appPointingButton->setEnabled(true);
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

	// image forwarding
	connect(ImageForwarder::get(),
		SIGNAL(offerImage(astro::image::ImagePtr, std::string)),
		this,
		SLOT(imageForSaving(astro::image::ImagePtr, std::string)));

	// add menu
	createActions();
	createMenus();

	// create a 
	QTimer::singleShot(1000, this, SLOT(timecheck()));
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
 * \brief Launch the Pointing subapplication
 */
void	MainWindow::launchPointing() {
	try {
		InstrumentSelectionApplication<snowgui::pointingwindow>	*is
			= new InstrumentSelectionApplication<snowgui::pointingwindow>(this, _serviceobject);
		is->setWindowTitle(QString("Select instrument for Pointing "
			"application"));
		is->exec();
		delete is;
	} catch (const std::exception& x) {
		QMessageBox	message;
		message.setText(QString("Cannot launch Pointing"));
		message.setInformativeText(QString(astro::stringprintf(
			"The Pointing subapplication could not be started. "
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
		windowsMenu->add(iw, QString("Instruments"));
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
		if (_configurationdialog) {
			_configurationdialog->raise();
			_configurationdialog->activateWindow();
		} else {
			_configurationdialog = new configurationdialog(NULL,
				_serviceobject);
			_configurationdialog->show();
			windowsMenu->add(_configurationdialog,
				QString("Configuration"));
			connect(_configurationdialog, SIGNAL(destroyed()),
                		this, SLOT(forgetConfiguration()));
		}
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

void	MainWindow::forgetConfiguration() {
	_configurationdialog = NULL;
}

/**
 * \brief Launch the Images subapplication
 */
void	MainWindow::launchImages() {
	try {
		imageswindow	*images = new imageswindow(NULL,
			_serviceobject);
		images->show();
		windowsMenu->add(images, QString("Images"));
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
		windowsMenu->add(rw, QString("Repositories"));
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

void	MainWindow::raiseMainwindow() {
	this->raise();
}

/**
 * \brief Create the actions in the menu
 */
void	MainWindow::createActions() {
	connectAction = new QAction(QString("Connect"), this);
	connect(connectAction, &QAction::triggered, this,
		&MainWindow::connectFile);

	openAction = new QAction(QString("Open Image"), this);
	connect(openAction, &QAction::triggered, this,
		&MainWindow::openFile);

	browseAction = new QAction(QString("Browse"), this);
	connect(browseAction, &QAction::triggered, this,
		&MainWindow::browseDirectory);

	saveAction = new QAction(QString("Save Image"), this);
	saveAction->setEnabled(false);
	connect(saveAction, &QAction::triggered, this,
		&MainWindow::saveImage);

	raiseAction = new QAction(QString("Main Window"), this);
	connect(raiseAction, &QAction::triggered, this,
		&MainWindow::raiseMainwindow);
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
	QMenu	*_windowsmenu = menuBar()->addMenu(QString("Windows"));
	_windowsmenu->addAction(raiseAction);
	windowsMenu = new WindowsMenu(_windowsmenu);
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
	case ServiceSubset::GATEWAY:
		return ui->gatewayLabel;
	}
	return NULL;
}

void	MainWindow::imageForSaving(astro::image::ImagePtr image,
		std::string imagestring) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "accepting image '%s'",
		imagestring.c_str());
	_image = image;
	_imagestring = imagestring;
	if (_image) {
		std::string     title("Save ");
		title = title + _imagestring + std::string(" image ");
                title = title + _image->size().toString();
                title = title + std::string("<");
                title = title + astro::demangle(_image->pixel_type().name());
                title = title + std::string(">");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new save title: '%s'",
			title.c_str());
		saveAction->setText(title.c_str());
		saveAction->setEnabled(true);
	} else {
		saveAction->setText("Save image");
		saveAction->setEnabled(false);
	}
}

/**
 * \brief Launch the Event monitoring subapp
 */
void	MainWindow::launchEvents() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launchEvents()");
	try {
		if (_eventdisplaywidget) {
			_eventdisplaywidget->raise();
			_eventdisplaywidget->activateWindow();
		} else {
			_eventdisplaywidget = new EventDisplayWidget(NULL,
							_serviceobject);
			_eventdisplaywidget->show();
			windowsMenu->add(_eventdisplaywidget,
				QString("Events"));
			connect(_eventdisplaywidget, SIGNAL(destroyed()),
				this, SLOT(forgetEvents()));
		}
	} catch (const std::exception& x) {
		QMessageBox	*message = new QMessageBox(this);
		message->setText(QString("Connection failure"));
		std::ostringstream	out;
		out << "Failed to connect to the 'Events' service on ";
		out << _serviceobject.toString();
		out << ". Repository window cannot be constructed.";
		out << "Cause: ";
		out << x.what();
		message->setInformativeText(QString(out.str().c_str()));
		message->exec();
		delete message;
	}
}

void	MainWindow::forgetEvents() {
	_eventdisplaywidget = NULL;
}

/**
 * \brief Test the time 
 */
void	MainWindow::timecheck() {
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
		_serviceobject.connect("Daemon"));
	snowstar::DaemonPrx	daemon
		= snowstar::DaemonPrx::checkedCast(base);
	time_t	servertime = daemon->getSystemTime();
	time_t	now;
	time(&now);
	int	delta = now - servertime;
	if (abs(delta) < 60) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "time difference %d", delta);
		return;
	}
	QMessageBox	message;
	message.setText("Time Problem");
	std::ostringstream	out;
	out << "There is a large time difference of " << abs(delta);
	out << " seconds between the client ";
	out << "and the server machine. Use the Configuration app to ";
	out << "sync the time.";
	message.setInformativeText(QString(out.str().c_str()));
	QPushButton	*_configbutton = message.addButton(QString("Configure"),
		QMessageBox::ButtonRole::AcceptRole);
	connect(_configbutton, SIGNAL(clicked()),
		this, SLOT(launchConfiguration()));
	message.addButton(QString("Ignore"), QMessageBox::ButtonRole::RejectRole);
	message.exec();
}

} // namespace snowgui

