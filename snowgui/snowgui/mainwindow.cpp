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

using namespace astro::discover;

MainWindow::MainWindow(QWidget *parent,
	const astro::discover::ServiceObject serviceobject)
	: QMainWindow(parent), _serviceobject(serviceobject),
	  ui(new Ui::MainWindow) {
	// create user interface components
	ui->setupUi(this);

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
	ui->appPreviewButton->setEnabled(true);

	// add menu
	createActions();
	createMenus();
}

void	MainWindow::launchPreview() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "launch a preview subapplication");
	InstrumentSelectionDialog	*is
		= new InstrumentSelectionDialog(this, _serviceobject);
	is->setWindowTitle(QString("Select instrument for Preview application"));
	is->exec();
	delete is;
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
    delete ui;
}
