/*
 * focusinghistorywidget.cpp -- focusing history widget implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "focusinghistorywidget.h"
#include "ui_focusinghistorywidget.h"

namespace snowgui {

focusinghistorywidget::focusinghistorywidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::focusinghistorywidget) {
	ui->setupUi(this);

	connect(ui->positionRadioButton, SIGNAL(clicked(bool)),
		this, SLOT(byPosition(bool)));
	connect(ui->sequenceRadioButton, SIGNAL(clicked(bool)),
		this, SLOT(bySequence(bool)));
	connect(ui->clearButton, SIGNAL(clicked()),
		this, SLOT(clear()));
	connect(ui->focuspointsWidget, SIGNAL(positionSelected(int)),
		this, SLOT(didSelectPosition(int)));
}

focusinghistorywidget::~focusinghistorywidget() {
	delete ui;
}

void	focusinghistorywidget::add(astro::image::ImagePtr image,
		unsigned short position) {
	ui->focuspointsWidget->add(image, position);
}

void	focusinghistorywidget::clear() {
	ui->focuspointsWidget->clear();
}

void	focusinghistorywidget::byPosition(bool b) {
	if (!b) { return; }
	ui->focuspointsWidget->setByPosition(true);
}

void	focusinghistorywidget::bySequence(bool b) {
	if (!b) { return; }
	ui->focuspointsWidget->setByPosition(false);
}

void	focusinghistorywidget::didSelectPosition(int p) {
	emit positionSelected(p);
}

} // namespace snowgui
