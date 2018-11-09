/*
 * skydisplaywidget.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "skydisplaydialog.h"
#include "ui_skydisplaydialog.h"
#include "SkyDisplayWidget.h"
#include <QKeyEvent>

namespace snowgui {

SkyDisplayDialog::SkyDisplayDialog(QWidget *parent)
	: QDialog(parent), ui(new Ui::SkyDisplayDialog) {
	ui->setupUi(this);
	setWindowTitle(QString("Current Sky View"));

	ui->skydisplayWidget->target_enabled(true);

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

void    SkyDisplayDialog::keyPressEvent(QKeyEvent *e) {
        if (e->key() == Qt::Key_Escape) {
                return;
        }
        QWidget::keyPressEvent(e);
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

void	SkyDisplayDialog::targetChanged(astro::RaDec target) {
	ui->skydisplayWidget->show_target(true);
	ui->skydisplayWidget->targetChanged(target);
}

void	SkyDisplayDialog::update() {
	ui->skydisplayWidget->update();
}

} // namespace snowgui
