/*
 * guidingwindow.cpp -- implementation of guiding window
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "guidingwindow.h"
#include "ui_guidingwindow.h"

namespace snowgui {

guidingwindow::guidingwindow(QWidget *parent) : InstrumentWidget(parent),
	ui(new Ui::guidingwindow) {
	ui->setupUi(this);
}

guidingwindow::~guidingwindow() {
	delete ui;
}

void	guidingwindow::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	ui->ccdcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->coolercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->focusercontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->filterwheelcontrollerWidget->instrumentSetup(serviceobject, instrument);
	ui->guiderportcontrollerWidget->instrumentSetup(serviceobject, instrument);
}

} // namespace snowgui
