/*
 * exposurewidget.cpp -- ExposureWidget implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "exposurewidget.h"
#include "ui_exposurewidget.h"

using namespace astro::image;

/**
 * \brief Construct an ExposureWidget
 */
ExposureWidget::ExposureWidget(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::ExposureWidget)
{
    ui->setupUi(this);

	timechange = false;

	ui->shutterComboBox->addItem(QString("open"));
	ui->shutterComboBox->addItem(QString("closed"));
}

/**
 * \brief Destroy the ExposureWidet
 */
ExposureWidget::~ExposureWidget()
{
    delete ui;
}

/**
 * \brief Slot called when the time spinner changes
 *
 * This is used to ensure that time steps are essentially exponential
 */
void	ExposureWidget::timeChanged(double value) {
	if (timechange) {
		return;
	}
	timechange = true;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "value = %f, timeprevious = %f",
		value, timeprevious);
	double	stepvalue = ui->timeSpinBox->singleStep();
	if (value < timeprevious) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stepping down");
		if (timeprevious >= 120) {
			stepvalue = 60;
		} else {
			stepvalue /= 2;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "new stepvalue: %f",
				stepvalue);
			if (stepvalue < 0.001) {
				stepvalue = 0.001;
			}
			stepvalue = trunc(1000 * stepvalue) / 1000.;
			ui->timeSpinBox->setValue(stepvalue);
		}
		ui->timeSpinBox->setSingleStep(stepvalue);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "stepping up");
		if (value > 1) {
			value = trunc(value);
			ui->timeSpinBox->setValue(value);
		}
		if (value >= 60) {
			ui->timeSpinBox->setSingleStep(60);
		} else {
			ui->timeSpinBox->setSingleStep(value);
		}
	}
	timeprevious = ui->timeSpinBox->value();
	timechange = false;
}

/**
 * \brief Slot called when the subframe checkbox is toggled
 */
void	ExposureWidget::subframeToggled(bool state) {
	ui->originxField->setEnabled(state);
	ui->originyField->setEnabled(state);
	ui->widthField->setEnabled(state);
	ui->heightField->setEnabled(state);
	if (state) {
		ui->subframeCheckBox->setText(QString("enabled: partial frame"));
	} else {
		ui->subframeCheckBox->setText(QString("disabled: full frame"));
	}
}

/**
 * \brief Set the CCD
 */
void	ExposureWidget::setCcd(CcdPtr _ccd) {
	// remember the CCD (there might be some information we want to read
	// from it later)
	ccd = _ccd;

	// read the binning modes and add the options to the combo box
	const BinningSet&	set = ccd->getInfo().binningmodes;
	std::set<Binning>::const_iterator	bi;
	for (bi = set.begin(); bi != set.end(); bi++) {
		ui->binningComboBox->addItem(QString(bi->toString().c_str()));
	}

	// set the frame size 
	ImageRectangle	frame = ccd->getInfo().getFrame();
	ui->originxField->setText(QString().setNum(frame.origin.x));
	ui->originyField->setText(QString().setNum(frame.origin.y));
	ui->widthField->setText(QString().setNum(frame.size.getWidth()));
	ui->heightField->setText(QString().setNum(frame.size.getHeight()));

	// if the ccd has no shutter, disable it
	ui->shutterLabel->setEnabled(ccd->hasShutter());
	ui->shutterComboBox->setEnabled(ccd->hasShutter());
}

/**
 * \brief Read the exposure parameters from the fields
 */
Exposure	ExposureWidget::getExposure() {
	// The exposure structure we want to return
	Exposure	result;

	/* read information from the GUI */

	// subframe info
	bool	ok;
	if (ui->subframeCheckBox->isChecked()) {
		result.frame.origin.x = ui->originxField->text().toInt(&ok);
		result.frame.origin.y = ui->originyField->text().toInt(&ok);
		result.frame.size.setWidth(ui->widthField->text().toInt(&ok));
		result.frame.size.setHeight(ui->widthField->text().toInt(&ok));
	} else {
		result.frame = ccd->getInfo().getFrame();
	}

	// exposure time
	result.exposuretime = ui->timeSpinBox->value();

	// binning mode
	BinningSet::const_iterator	i = ccd->getInfo().binningmodes.begin();
	int	binning_entry = ui->binningComboBox->currentIndex();
	while (binning_entry-- > 0) { i++; }
	result.mode = *i;

	// shutter info
	if (ccd->hasShutter()) {
		switch (ui->shutterComboBox->currentIndex()) {
		case 0:
			result.shutter = SHUTTER_OPEN;
			break;
		case 1:
			result.shutter = SHUTTER_CLOSED;
			break;
		}
	} else {
		result.shutter = SHUTTER_OPEN;
	}

	// return the exposure
	return result;
}

/**
 * \brief display current exposure values are displayed
 */
void	ExposureWidget::setExposure(const Exposure& exposure) {
	// display exposure window parameters
	ui->originxField->setText(QString().setNum(exposure.frame.origin.x));
	ui->originyField->setText(QString().setNum(exposure.frame.origin.y));
	ui->widthField->setText(QString().setNum(exposure.frame.size.getWidth()));
	ui->heightField->setText(QString().setNum(exposure.frame.size.getHeight()));

	// normalize the exposure to so that it fits into the constraints of
	// the exposure time spinner
	double	exposuretime = exposure.exposuretime;
	exposuretime = trunc(1000. * exposuretime) / 1000.;
	if (exposuretime > 3600) {
		exposuretime = 3600;
	}
	if (exposuretime < 0.001) {
		exposuretime = 0.001;
	}
	ui->timeSpinBox->setValue(exposure.exposuretime);

	// find the right binning mode to display in the binning mode
	// combo box
	BinningSet::const_iterator	i;
	int	binning_entry = 0;
	for (i = ccd->getInfo().binningmodes.begin();
		i != ccd->getInfo().binningmodes.end(); i++) {
		if (exposure.mode == *i) {
			break;
		}
		binning_entry++;
	}
	ui->binningComboBox->setCurrentIndex(binning_entry);
}

