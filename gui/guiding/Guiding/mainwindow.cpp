#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <AstroDebug.h>
#include <guiderdialog.h>
#include <cstdlib>
#include <cstdio>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void	MainWindow::startGuider() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start guider");
	// create a new guider dialog
	GuiderDialog	*gd = new GuiderDialog(guider, this);
	Astro::GuiderDescriptor_var	descriptor = guider->getDescriptor();
	char	buffer[1024];
	snprintf(buffer, sizeof(buffer), "%s|%d|%s", &*(descriptor->cameraname),
		descriptor->ccdid, &*(descriptor->guiderportname));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "title: %s", buffer);
	gd->setWindowTitle(buffer);
	gd->show();
}
