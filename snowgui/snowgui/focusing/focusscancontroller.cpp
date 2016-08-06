/*
 * focusscancontroller.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "focusscancontroller.h"
#include "ui_focusscancontroller.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroImage.h>

using namespace astro::image;

namespace snowgui {

focusscancontroller::focusscancontroller(QWidget *parent) : QWidget(parent),
	ui(new Ui::focusscancontroller) {
	ui->setupUi(this);
	currentstep = -1;
	scanning = false;
	ui->scanProgress->setValue(0);
	connect(ui->scanButton, SIGNAL(clicked()), this, SLOT(scanClicked()));
}

focusscancontroller::~focusscancontroller() {
	delete ui;
}

void	focusscancontroller::setScanning(bool s) {
	if (s) {
		ui->centerBox->setEnabled(false);
		ui->nstepsBox->setEnabled(false);
		ui->stepBox->setEnabled(false);
		ui->scanButton->setText("Stop");
		ui->scanProgress->setMaximum(2 * numberofsteps + 2);
		ui->scanProgress->setValue(1);
		scanning = true;
	} else {
		scanning = false;
		ui->scanButton->setText("Scan");
		ui->centerBox->setEnabled(true);
		ui->nstepsBox->setEnabled(true);
		ui->stepBox->setEnabled(true);
		ui->scanProgress->setValue(0);
	}
}

void	focusscancontroller::startScan() {
	numberofsteps = ui->nstepsBox->value();
	int	centerposition = ui->centerBox->value();
	stepsize = ui->stepBox->value();
	currentstep = 0;
	position = centerposition - stepsize * (numberofsteps / 2);
	setScanning(true);
	std::string	status = astro::stringprintf(
		"start scan, move to position %d", position);
	ui->statusLabel->setText(QString(status.c_str()));
	emit	movetoPosition(position);
}

void	focusscancontroller::stopScan() {
	setScanning(false);
	std::string	status = astro::stringprintf(
			"scan stopped after %d steps", currentstep);
	ui->statusLabel->setText(QString(status.c_str()));
}

void	focusscancontroller::scanClicked() {
	if (scanning) {
		stopScan();
	} else {
		startScan();
	}
}

void	focusscancontroller::positionReached() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "position reached");
	if (!scanning) {
		return;
	}
	ui->scanProgress->setValue(2 * currentstep + 2);
	std::string	status = astro::stringprintf(
		"capture image %d @ position %d", currentstep + 1, position);
	ui->statusLabel->setText(QString(status.c_str()));
	emit performCapture();
}

void	focusscancontroller::imageReceived(ImagePtr /* image */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image received");
	if (!scanning) {
		return;
	}
	currentstep++;
	ui->scanProgress->setValue(2 * currentstep + 1);
	if (currentstep > numberofsteps) {
		ui->statusLabel->setText(QString("scan complete"));
		setScanning(false);
		return;
	}
	position += stepsize;
	std::string	status = astro::stringprintf(
		"moving to position %d", position);
	ui->statusLabel->setText(QString(status.c_str()));
	emit movetoPosition(position);
}

} // namespace snowgui
