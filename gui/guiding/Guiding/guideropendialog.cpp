/*
 * guideropendialog.cpp -- implementation of guider open dialog
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "guideropendialog.h"
#include "ui_guideropendialog.h"
#include <connectiondialog.h>
#include <guider.hh>
#include <module.hh>
#include <stdexcept>
#include <AstroDebug.h>
#include <guiderwidget.h>

/**
 * \brief Create a GuiderOpenDialog 
 */
GuiderOpenDialog::GuiderOpenDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GuiderOpenDialog)
{
	ui->setupUi(this);

	// create the name
        CosNaming::Name name;
        name.length(2);
        name[0].id = "Astro";
        name[0].kind = "context";
        name[1].id = "Modules";
        name[1].kind = "object";

	// resolve the Modules name
        CORBA::Object_var	obj
		= ConnectionDialog::namingcontext->resolve(name);
        Astro::Modules_var	modules = Astro::Modules::_narrow(obj);
        if (CORBA::is_nil(modules)) {
                throw std::runtime_error("nil object reference");
        }
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a Modules object");

	// now tell the camera combo box to retrieve a list of all cameras
	ui->cameraBox->set(modules,
		Astro::DeviceLocator::DEVICE_CAMERA);
	ui->guiderportBox->set(modules,
		Astro::DeviceLocator::DEVICE_GUIDERPORT);
}

/**
 * \brief Destroy a GuiderOpenDialog
 */
GuiderOpenDialog::~GuiderOpenDialog()
{
	delete ui;
}

/**
 * \brief what to do when the configuration is accepted
 */
void	GuiderOpenDialog::accept() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start guider");

	// set up the name of the guider factory
	CosNaming::Name	name;
	name.length(2);
	name[0].id = "Astro";
	name[0].kind = "context";
	name[1].id = "GuiderFactory";
	name[1].kind = "object";

	// get a guiderfactory reference
        CORBA::Object_var	obj
		= ConnectionDialog::namingcontext->resolve(name);
	Astro::GuiderFactory_var	guiderfactory
		= Astro::GuiderFactory::_narrow(obj);
	if (CORBA::is_nil(guiderfactory)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get root context");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a guider factory reference");

	// create a new guider dialog
	Astro::GuiderDescriptor	*gd = new Astro::GuiderDescriptor();
	Astro::GuiderDescriptor_var	gdvar = gd;

	int	cameraindex = ui->cameraBox->currentIndex();
	QByteArray	bacamera
		= ui->cameraBox->itemText(cameraindex).toLocal8Bit();
	gd->cameraname = CORBA::string_dup(bacamera.data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera selected: %s",
		(char *)gd->cameraname);

	gd->ccdid = ui->ccdSpinbox->value();

	int	guiderportindex = ui->guiderportBox->currentIndex();
	QByteArray	baguiderport
		= ui->guiderportBox->itemText(guiderportindex).toLocal8Bit();
	gd->guiderportname = CORBA::string_dup(baguiderport.data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guiderport selected: %s",
		(char *)gd->guiderportname);

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

	GuiderWidget	*guiderwidget = new GuiderWidget(guider);
	guiderwidget->show();

	// close this dialog
	close();
}

