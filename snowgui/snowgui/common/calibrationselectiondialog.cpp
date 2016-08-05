#include "calibrationselectiondialog.h"
#include "ui_calibrationselectiondialog.h"

calibrationselectiondialog::calibrationselectiondialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::calibrationselectiondialog)
{
    ui->setupUi(this);
}

calibrationselectiondialog::~calibrationselectiondialog()
{
    delete ui;
}
