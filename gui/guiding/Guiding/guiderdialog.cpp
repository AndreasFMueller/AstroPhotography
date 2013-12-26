#include "guiderdialog.h"
#include "ui_guiderdialog.h"
#include <AstroDebug.h>

GuiderDialog::GuiderDialog(Astro::Guider_var guider, QWidget *parent) :
    _guider(guider), QDialog(parent),
    ui(new Ui::GuiderDialog)
{
    ui->setupUi(this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider dialog created");
}

GuiderDialog::~GuiderDialog()
{
    delete ui;
}
