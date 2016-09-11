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
#include <previewwindow.h>
#include <focusingwindow.h>
#include <guidingwindow.h>
#include <instrumentswindow.h>
#include "configurationdialog.h"
#include <imageswindow.h>
#include <repositorywindow.h>
#include <QMessageBox>
#include <sstream>
#include <exposewindow.h>

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
		InstrumentSelectionApplication<snowgui::PreviewWindow>	*is
			= new InstrumentSelectionApplication<snowgui::PreviewWindow>(this, _serviceobject);
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
		out << ". Images window cannot be constructed.";
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
	QMessageBox	*messagebox = new QMessageBox(this);
	messagebox->setText(QString("Application not implemented"));
	messagebox->setInformativeText(QString("The Tasks application is not "
		"yet implemented"));
	messagebox->exec();
	delete messagebox;
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
 * \brief Create the actions in the menu
 */
void	MainWindow::createActions() {
	connectAction = new QAction(QString("connect"), this);
	connect(connectAction, &QAction::triggered, this, &MainWindow::connectFile);
}

/**
 * \brief Create the menus of the main window
 */
void	MainWindow::createMenus() {
	fileMenu = menuBar()->addMenu(QString("File"));
	fileMenu->addAction(connectAction);
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

} // namespace snowgui
