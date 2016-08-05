/*
 * calibrationwidget.cpp -- calibration widget implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "calibrationwidget.h"
#include "ui_calibrationwidget.h"

namespace snowgui {

calibrationwidget::calibrationwidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::calibrationwidget)
{
	ui->setupUi(this);
}

calibrationwidget::~calibrationwidget()
{
	delete ui;
}

} // namespace snowgui
