#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include <AstroDebug.h>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);
}

ConnectionDialog::~ConnectionDialog() 
{
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy connection dialog");
    delete ui;
}
