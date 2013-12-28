#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <AstroDebug.h>
#include <guiderdialog.h>
#include <cstdlib>
#include <cstdio>
#include <QByteArray>

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
	Astro::GuiderDescriptor	*gd = new Astro::GuiderDescriptor();
	Astro::GuiderDescriptor_var	gdvar = gd;
	QByteArray	bacamera = ui->cameraField->text().toLocal8Bit();
	gd->cameraname = CORBA::string_dup(bacamera.data());
	gd->ccdid = ui->ccdSpinbox->value();
	QByteArray	baguiderport = ui->guiderportField->text().toLocal8Bit();
	gd->guiderportname = CORBA::string_dup(baguiderport.data());

	// now go after the guider
	Astro::Guider_var	guider;
	try {
		guider = guiderfactory->get(*gd);
		if (CORBA::is_nil(guider)) {
			debug(LOG_ERR, DEBUG_LOG, 0, "no guider obtained");
			return;
		}
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "guider request failed");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider reference obtained");

	GuiderDialog	*guiderdialog = new GuiderDialog(guider, this);
	guiderdialog->show();
}
