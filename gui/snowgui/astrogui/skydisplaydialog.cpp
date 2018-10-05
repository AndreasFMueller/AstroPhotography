/*
 * skydisplaywidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "skydisplaydialog.h"
#include "ui_skydisplaydialog.h"
#include "SkyDisplayWidget.h"

namespace snowgui {

SkyDisplayDialog::SkyDisplayDialog(QWidget *parent)
	: QDialog(parent), ui(new Ui::SkyDisplayDialog) {
	ui->setupUi(this);
	setWindowTitle(QString("Current Sky View"));

	// initial state
	ui->azmaltCheckBox->setCheckState(ui->skydisplayWidget->show_altaz()
		? Qt::Checked : Qt::Unchecked);
	ui->radecCheckBox->setCheckState(ui->skydisplayWidget->show_radec()
		? Qt::Checked : Qt::Unchecked);
	ui->constellationsCheckBox->setCheckState(
		ui->skydisplayWidget->show_constellations()
			? Qt::Checked : Qt::Unchecked);
	ui->telescopeCheckBox->setCheckState(
		ui->skydisplayWidget->show_telescope()
			? Qt::Checked : Qt::Unchecked);
	ui->targetCheckBox->setCheckState(ui->skydisplayWidget->show_target()
		? Qt::Checked : Qt::Unchecked);
	ui->labelsCheckBox->setCheckState(ui->skydisplayWidget->show_labels()
		? Qt::Checked : Qt::Unchecked);

	// connections
	connect(ui->azmaltCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(altazChanged(int)));
	connect(ui->radecCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(radecChanged(int)));
	connect(ui->constellationsCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(constellationsChanged(int)));
	connect(ui->telescopeCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(telescopeChanged(int)));
	connect(ui->targetCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(targetChanged(int)));
	connect(ui->labelsCheckBox, SIGNAL(stateChanged(int)),
		this, SLOT(labelsChanged(int)));

	// connect the
	connect(ui->skydisplayWidget, SIGNAL(pointSelected(astro::RaDec)),
		this, SLOT(targetSelected(astro::RaDec)));
}

SkyDisplayDialog::~SkyDisplayDialog() {
	delete ui;
}

const astro::RaDec&	SkyDisplayDialog::telescope() const {
	return ui->skydisplayWidget->telescope();
}

void	SkyDisplayDialog::telescope(const astro::RaDec& t) {
	ui->skydisplayWidget->telescope(t);
}

const astro::RaDec&	SkyDisplayDialog::target() const {
	return ui->skydisplayWidget->target();
}

void	SkyDisplayDialog::target(const astro::RaDec& t) {
	ui->skydisplayWidget->target(t);
}

void	SkyDisplayDialog::position(const astro::LongLat& l) {
	ui->skydisplayWidget->position(l);
}

const astro::LongLat&	SkyDisplayDialog::position() const {
	return ui->skydisplayWidget->position();
}

void	SkyDisplayDialog::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

void	SkyDisplayDialog::telescopeChanged(astro::RaDec radec) {
	ui->skydisplayWidget->telescopeChanged(radec);
}

void	SkyDisplayDialog::positionChanged(astro::LongLat longlat) {
	ui->skydisplayWidget->positionChanged(longlat);
}

void	SkyDisplayDialog::targetSelected(astro::RaDec radec) {
	emit pointSelected(radec);
}

void	SkyDisplayDialog::altazChanged(int state) {
	bool	checked = (state == Qt::Checked);
	ui->skydisplayWidget->show_altaz(checked);
	ui->skydisplayWidget->update();
}

void	SkyDisplayDialog::radecChanged(int state) {
	bool	checked = (state == Qt::Checked);
	ui->skydisplayWidget->show_radec(checked);
	ui->skydisplayWidget->update();
}

void	SkyDisplayDialog::constellationsChanged(int state) {
	bool	checked = (state == Qt::Checked);
	ui->skydisplayWidget->show_constellations(checked);
	ui->skydisplayWidget->update();
}

void	SkyDisplayDialog::telescopeChanged(int state) {
	bool	checked = (state == Qt::Checked);
	ui->skydisplayWidget->show_telescope(checked);
	ui->skydisplayWidget->update();
}

void	SkyDisplayDialog::targetChanged(int state) {
	bool	checked = (state == Qt::Checked);
	ui->skydisplayWidget->show_target(checked);
	ui->skydisplayWidget->update();
}

void	SkyDisplayDialog::targetChanged(astro::RaDec target) {
	ui->skydisplayWidget->targetChanged(target);
}

void	SkyDisplayDialog::labelsChanged(int state) {
	bool	checked = (state == Qt::Checked);
	ui->skydisplayWidget->show_labels(checked);
	ui->skydisplayWidget->update();
}

void	SkyDisplayDialog::update() {
	ui->skydisplayWidget->update();
}

} // namespace snowgui
