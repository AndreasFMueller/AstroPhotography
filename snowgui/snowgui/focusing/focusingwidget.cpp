/*
 * focusingwidget.cpp -- implementation of the focusing widget
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "focusingwidget.h"
#include "ui_focusingwidget.h"

focusingwidget::focusingwidget(QWidget *parent)
	: snowgui::InstrumentWidget(parent), ui(new Ui::focusingwidget) {
	ui->setupUi(this);
	ui->imageWidget->setInfoVisible(false);
}

focusingwidget::~focusingwidget() {
	delete ui;
}

void	focusingwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);
	ui->ccdcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->coolercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->focusercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->filterwheelcontrollerWidget->instrumentSetup(serviceobject, instrument);
}
