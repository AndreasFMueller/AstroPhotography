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

#define EXPOSURE_MIN	0.0001

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "initializing CCD properties");
	// remember the CCD (there might be some information we want to read
	// from it later)
	ccd = _ccd;

	// read the binning modes and add the options to the combo box
	const BinningSet&	set = ccd->getInfo().modes();
	std::set<Binning>::const_iterator	bi;
	for (bi = set.begin(); bi != set.end(); bi++) {
		ui->binningComboBox->addItem(QString(bi->toString().c_str()));
	}

	// set the frame size 
	ImageRectangle	frame = ccd->getInfo().getFrame();
	ui->originxField->setText(QString().setNum(frame.origin().x()));
	ui->originyField->setText(QString().setNum(frame.origin().y()));
	ui->widthField->setText(QString().setNum(frame.size().width()));
	ui->heightField->setText(QString().setNum(frame.size().height()));

	// if the ccd has no shutter, disable it
	ui->shutterLabel->setEnabled(ccd->hasShutter());
	ui->shutterComboBox->setEnabled(ccd->hasShutter());

	// if the ccd has a gain setting, configureit
	if (ccd->hasGain()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has gain control");
		ui->gainLabel->setEnabled(true);
		ui->gainSlider->setEnabled(true);
		std::pair<float, float>	gaininterval = ccd->gainInterval();
		mingain = gaininterval.first;
		maxgain = gaininterval.second;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "gain interval: [%f,%f]",
			mingain, maxgain);
		gainunit = (maxgain - mingain) / 100.;
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has no gain control");
	}
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
		int	originx = ui->originxField->text().toInt(&ok);
		int	originy = ui->originyField->text().toInt(&ok);
		result.frame.setOrigin(ImagePoint(originx, originy));

		int	sizex = ui->widthField->text().toInt(&ok);
		int	sizey = ui->heightField->text().toInt(&ok);
		result.frame.setSize(ImageSize(sizex, sizey));
	} else {
		result.frame = ccd->getInfo().getFrame();
	}

	// exposure time
	result.exposuretime = ui->timeSpinBox->value();
	if (result.exposuretime < 0.0001) {
		result.exposuretime = 0.0001;
	}

	// binning mode
	BinningSet::const_iterator	i = ccd->getInfo().modes().begin();
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

	// read the gain value
	if (ccd->hasGain()) {
		result.gain = mingain
			+ ui->gainSlider->sliderPosition() * gainunit;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "gain is %.3f", result.gain);
	}

	// return the exposure
	return result;
}

/**
 * \brief display current exposure values are displayed
 */
void	ExposureWidget::setExposure(const Exposure& exposure) {
	// display exposure window parameters
	ui->originxField->setText(QString().setNum(exposure.frame.origin().x()));
	ui->originyField->setText(QString().setNum(exposure.frame.origin().y()));
	ui->widthField->setText(QString().setNum(exposure.frame.size().width()));
	ui->heightField->setText(QString().setNum(exposure.frame.size().height()));

	// normalize the exposure to so that it fits into the constraints of
	// the exposure time spinner
	double	exposuretime = exposure.exposuretime;
	exposuretime = trunc(1000. * exposuretime) / 1000.;
	if (exposuretime > 3600) {
		exposuretime = 3600;
	}
	if (exposuretime < EXPOSURE_MIN) {
		exposuretime = EXPOSURE_MIN;
	}
	ui->timeSpinBox->setValue(exposure.exposuretime);

	// find the right binning mode to display in the binning mode
	// combo box
	BinningSet	modes = ccd->getInfo().modes();
	BinningSet::const_iterator	i;
	int	binning_entry = 0;
	for (i = modes.begin(); i != modes.end(); i++) {
		if (exposure.mode == *i) {
			break;
		}
		binning_entry++;
	}
	ui->binningComboBox->setCurrentIndex(binning_entry);
}

