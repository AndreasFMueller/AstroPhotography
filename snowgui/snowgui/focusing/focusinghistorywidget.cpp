/*
 * focusinghistorywidget.cpp -- focusing history widget implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "focusinghistorywidget.h"
#include "ui_focusinghistorywidget.h"

namespace snowgui {

/**
 * \brief Create a n ew focusing history widget
 */
focusinghistorywidget::focusinghistorywidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::focusinghistorywidget) {
	ui->setupUi(this);

	connect(ui->positionRadioButton, SIGNAL(clicked(bool)),
		this, SLOT(byPosition(bool)));
	connect(ui->sequenceRadioButton, SIGNAL(clicked(bool)),
		this, SLOT(bySequence(bool)));

	connect(ui->measureFWHMButton, SIGNAL(clicked(bool)),
		this, SLOT(useFWHM(bool)));
	connect(ui->measureBrennerButton, SIGNAL(clicked(bool)),
		this, SLOT(useBrenner(bool)));

	connect(ui->clearButton, SIGNAL(clicked()),
		this, SLOT(clear()));
	connect(ui->focuspointsWidget, SIGNAL(positionSelected(int)),
		this, SLOT(didSelectPosition(int)));
}

/**
 * \brief Destroy the focusing history
 */
focusinghistorywidget::~focusinghistorywidget() {
	delete ui;
}

/**
 * \brief Add a new image and position
 *
 * This method hands the widget over to the focuspointsWidget, which does
 * the actual computation and display
 */
void	focusinghistorywidget::add(astro::image::ImagePtr image,
		unsigned short position) {
	ui->focuspointsWidget->add(image, position);
}

/**
 * \brief Remove all points from the history
 */
void	focusinghistorywidget::clear() {
	ui->focuspointsWidget->clear();
}

/**
 * \brief make sure the focuspointsWidget uses position to sort the points
 */
void	focusinghistorywidget::byPosition(bool b) {
	if (!b) { return; }
	ui->focuspointsWidget->setByPosition(true);
}

/**
 * /brief Request that focuspointswidget use sequence to sort the points
 */
void	focusinghistorywidget::bySequence(bool b) {
	if (!b) { return; }
	ui->focuspointsWidget->setByPosition(false);
}

/**
 * \brief Slot called when a position was selected in the focuspointsWidget
 */
void	focusinghistorywidget::didSelectPosition(int p) {
	emit positionSelected(p);
}

/**
 *\brief make sure the focuspointsWidgets uses Brenner measure
 */
void	focusinghistorywidget::useBrenner(bool b) {
	if (!b) { return; }
	ui->focuspointsWidget->setUseFWHM(true);
}

/**
 *\brief make sure the focuspointsWidgets uses FWHM measure
 */
void	focusinghistorywidget::useFWHM(bool b) {
	if (!b) { return; }
	ui->focuspointsWidget->setUseFWHM(false);
}

} // namespace snowgui
