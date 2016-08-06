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
#include <QMessageBox>

using namespace astro::discover;

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
	connect(ui->appRepositoryButton, SIGNAL(clicked()),
		this, SLOT(launchTasks()));

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

	// add menu
	createActions();
	createMenus();
}

void	MainWindow::launchPreview() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch a preview subapplication");
	InstrumentSelectionApplication<snowgui::PreviewWindow>	*is
		= new InstrumentSelectionApplication<snowgui::PreviewWindow>(this, _serviceobject);
	is->setWindowTitle(QString("Select instrument for Preview application"));
	is->exec();
	delete is;
}

void	MainWindow::launchFocusing() {
	InstrumentSelectionApplication<snowgui::focusingwindow>	*is
		= new InstrumentSelectionApplication<snowgui::focusingwindow>(this, _serviceobject);
	is->setWindowTitle(QString("Select instrument for Focusing application"));
	is->exec();
	delete is;
}

void	MainWindow::launchGuiding() {
	InstrumentSelectionApplication<snowgui::guidingwindow>	*is
		= new InstrumentSelectionApplication<snowgui::guidingwindow>(this, _serviceobject);
	is->setWindowTitle(QString("Select instrument for Guiding application"));
	is->exec();
	delete is;
}

void	MainWindow::launchInstruments() {
	QMessageBox	*messagebox = new QMessageBox(this);
	messagebox->setText(QString("Application not implemented"));
	messagebox->setInformativeText(QString("The Instruments application is not yet implemented"));
	messagebox->exec();
	delete messagebox;
}

void	MainWindow::launchRepository() {
	QMessageBox	*messagebox = new QMessageBox(this);
	messagebox->setText(QString("Application not implemented"));
	messagebox->setInformativeText(QString("The Repository application is not yet implemented"));
	messagebox->exec();
	delete messagebox;
}

void	MainWindow::launchTasks() {
	QMessageBox	*messagebox = new QMessageBox(this);
	messagebox->setText(QString("Application not implemented"));
	messagebox->setInformativeText(QString("The Tasks application is not yet implemented"));
	messagebox->exec();
	delete messagebox;
}

void	MainWindow::connectFile() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connect action invoked");

	ServiceDiscoveryPtr	servicediscovery = ServiceDiscovery::get();
	sleep(1);
	ServerSelectionDialog	*sd = new ServerSelectionDialog(this,
		servicediscovery);
	sd->show();
}

void	MainWindow::createActions() {
	connectAction = new QAction(QString("connect"), this);
	connect(connectAction, &QAction::triggered, this, &MainWindow::connectFile);
}

void	MainWindow::createMenus() {
	fileMenu = menuBar()->addMenu(QString("File"));
	fileMenu->addAction(connectAction);
}

void	MainWindow::setServiceLabelEnabled(ServiceSubset::service_type t) {
	QLabel	*l = serviceLabel(t);
	if (_serviceobject.has(t)) {
		l->setStyleSheet("QLabel { background-color : white; color : black; }");
	} else {
		l->setStyleSheet("QLabel { background-color : transparent; color : grey; }");
	}
}

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

MainWindow::~MainWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy MainWindow");
	delete ui;
}
