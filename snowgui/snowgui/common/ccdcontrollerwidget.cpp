/*
 * ccdcontrollerwidget.cpp --  ccd controller implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "ccdcontrollerwidget.h"
#include "ui_ccdcontrollerwidget.h"
#include <IceConversions.h>
#include <algorithm>

using namespace astro::image;
using namespace astro::camera;

namespace snowgui {

/**
 * \brief Constructor for the CCD controller
 */
ccdcontrollerwidget::ccdcontrollerwidget(QWidget *parent) :
	InstrumentWidget(parent), ui(new Ui::ccdcontrollerwidget) {
	// setup user interface components
	ui->setupUi(this);

}

void	ccdcontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about CCDs available on this instrument, and 
	// remember the first ccd you can find
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCCD, index)) {
		snowstar::CcdPrx	ccd = _instrument.ccd(index);
		if (!_ccd) {
			_ccd = ccd;
		}
		ui->ccdSelectionBox->addItem(QString(ccd->getName().c_str()));
		index++;
	}
	index = 0;
	while (_instrument.has(snowstar::InstrumentGuiderCCD, index)) {
		snowstar::CcdPrx	ccd = _instrument.ccd(index);
		if (!_ccd) {
			_ccd = ccd;
		}
		ui->ccdSelectionBox->addItem(QString(ccd->getName().c_str()));
		index++;
	}

	// add additional information about this ccd
	setupCcd();
}

/**
 * \brief Destroy the CCD controller
 */
ccdcontrollerwidget::~ccdcontrollerwidget() {
	delete ui;
}

/**
 * \brief Read information from the ccd and show it
 */
void	ccdcontrollerwidget::setupCcd() {
	// make sure no signals are sent while setting up the CCD
	ui->binningSelectionBox->setEnabled(false);
	ui->binningSelectionBox->blockSignals(true);

	// remove all entry from the binning mode combobox
	while (ui->binningSelectionBox->count() > 0) {
		ui->binningSelectionBox->removeItem(0);
	}

	// propagate the information from the ccdinfo structure
	if (_ccd) {
		_ccdinfo = _ccd->getInfo();
		QComboBox	*binBox = ui->binningSelectionBox;
		CcdInfo	ccdinfo(convert(_ccdinfo));
		std::for_each(ccdinfo.modes().begin(), ccdinfo.modes().end(),
			[binBox](const Binning& mode) {
				std::string	m = astro::stringprintf("%dx%d",
					mode.x(), mode.y());
				binBox->addItem(QString(m.c_str()));
			}
		);

		// get pixel size information from Ccd
		std::string	ccdinfotext 
			= astro::stringprintf("%d x %d (%.1fµm x %.1fµm)",
				ccdinfo.size().width(), ccdinfo.size().height(),
				1000000 * ccdinfo.pixelwidth(),
				1000000 * ccdinfo.pixelheight());
		ui->sizeInfoField->setText(QString(ccdinfotext.c_str()));

		// use the frame size as the default rectangle
		displayFrame(ImageRectangle(ccdinfo.size()));
	}

	// now reenable signals
	ui->binningSelectionBox->blockSignals(false);
	ui->binningSelectionBox->setEnabled(true);
}

/**
 * \brief Display the settings in the argument exposure 
 *
 * This method does not send any signals
 */
void	ccdcontrollerwidget::displayExposure(Exposure e) {
	displayBinning(e.mode());
	displayExposureTime(e.exposuretime());
	displayPurpose(e.purpose());
	displayShutter(e.shutter());
}

/**
 * \brief This is the slot to call when setting an exposure from the outside
 *
 * This method ensures that all the GUI elements are updated, and then sends 
 * the exposureChanged() signal to whatever widget is connected to it.
 */
void	ccdcontrollerwidget::setExposure(Exposure e) {
	if (_exposure != e) {
		return;
	}
	displayExposure(_exposure);
	emit exposureChanged(_exposure);
}

/**
 * \brief Display new frame settings
 *
 * This method ensures that the GUI element and the internal state are in
 * sync, but it does not send any signals.
 */
void	ccdcontrollerwidget::displayFrame(ImageRectangle r) {
	// is the rectangle contained in the ccd
	if (!convert(_ccdinfo).size().bounds(r)) {
		return;
	}
	_exposure.frame(r);
	ui->frameSizeWidth->blockSignals(true);
	ui->frameSizeWidth->setText(
		QString(astro::stringprintf("%d", r.size().width()).c_str()));
	ui->frameSizeWidth->blockSignals(false);
	ui->frameSizeHeight->blockSignals(true);
	ui->frameSizeHeight->setText(
		QString(astro::stringprintf("%d", r.size().height()).c_str()));
	ui->frameSizeHeight->blockSignals(false);
	ui->frameOriginX->blockSignals(true);
	ui->frameOriginX->setText(
		QString(astro::stringprintf("%d", r.origin().x()).c_str()));
	ui->frameOriginX->blockSignals(false);
	ui->frameOriginY->blockSignals(true);
	ui->frameOriginY->setText(
		QString(astro::stringprintf("%d", r.origin().y()).c_str()));
	ui->frameOriginY->blockSignals(false);
}

/**
 * \brief Change the subframe rectangle
 *
 * This slot sends the exposureChanged signal
 */
void	ccdcontrollerwidget::setFrame(ImageRectangle r) {
	if (_exposure.frame() == r) {
		return;
	}
	displayFrame(r);
	emit exposureChanged(_exposure);
}

/**
 * \brief Display the new binning mode
 *
 * This method does not send signals
 */
void	ccdcontrollerwidget::displayBinning(Binning b) {
	// is binning mode supported by this camera?
	if (!convert(_ccdinfo).modes().permits(b)) {
		// XXX should we send an exeption here?
		return;
	}
	_exposure.mode(b);
	ui->binningSelectionBox->blockSignals(true);
	QString	modestring(b.toString().c_str());
	for (int i = 0; i < ui->binningSelectionBox->count(); i++) {
		if (modestring == ui->binningSelectionBox->itemText(i)) {
			ui->binningSelectionBox->setCurrentIndex(i);
			break;
		}
	}
	ui->binningSelectionBox->blockSignals(false);
}

/**
 */
Binning	ccdcontrollerwidget::getBinning(int index) {
	if ((index >= ui->binningSelectionBox->count()) && (index < 0)) {
		std::string	msg = astro::stringprintf("invalid binning "
			"index: %d", index);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::range_error(msg);
	}
	Binning	b(ui->binningSelectionBox->itemText(index).toLatin1().data());
	return b;
}

/**
 * \brief Display the binning mode based on the index
 */
void	ccdcontrollerwidget::displayBinning(int index) {
	displayBinning(getBinning(index));
}

/**
 * \brief Set the binning mode
 *
 * This method displays the new binning mode (if it is acceptable, i.e.
 * supported by the CCD in use), and then sends the exposureChanged signal.
 */
void	ccdcontrollerwidget::setBinning(Binning b) {
	if (_exposure.mode() == b) {
		return;
	}
	displayBinning(b);
	emit exposureChanged(_exposure);
}

/**
 * \brief Display the exposure time
 *
 * This method does not send any signals
 */
void	ccdcontrollerwidget::displayExposureTime(double t) {
	_exposure.exposuretime(t);
	ui->exposureSpinBox->blockSignals(true);
	ui->exposureSpinBox->setValue(t);
	ui->exposureSpinBox->blockSignals(false);
}

/**
 * \brief Set the exposure time
 */
void	ccdcontrollerwidget::setExposureTime(double t) {
	displayExposureTime(t);
	emit exposureChanged(_exposure);
}

/**
 * \brief Get the purpose from the menu index
 */
Exposure::purpose_t	ccdcontrollerwidget::getPurpose(int index) {
	switch (index) {
	case 0:	return Exposure::light;
	case 1:	return Exposure::dark;
	case 2:	return Exposure::flat;
	case 3:	return Exposure::bias;
	case 4:	return Exposure::test;
	case 5:	return Exposure::guide;
	case 6:	return Exposure::focus;
	default:
		break;
	}
	std::string	msg = astro::stringprintf("invalid purpose index: %d",
		index);
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::range_error(msg);
}

/**
 * \brief Display the new purpose
 *
 * This method does not send any signals
 */
void	ccdcontrollerwidget::displayPurpose(Exposure::purpose_t p) {
	_exposure.purpose(p);
	ui->purposeBox->blockSignals(true);
	ui->purposeBox->setCurrentIndex((int)p);
	ui->purposeBox->blockSignals(false);
}

/**
 * \brief Set a new purpose for the next exposure
 *
 * This method sets the new purpose and then sends the exposureChanged()
 * signal.
 */
void	ccdcontrollerwidget::setPurpose(Exposure::purpose_t p) {
	if (_exposure.purpose() == p) {
		return;
	}
	displayPurpose(p);
	emit exposureChanged(_exposure);
}

/**
 * \brief Display the new shutter state
 *
 * This method does not send any signals
 */
void	ccdcontrollerwidget::displayShutter(Shutter::state s) {
	_exposure.shutter(s);
	ui->shutterOpenBox->blockSignals(true);
	if (_exposure.shutter() == Shutter::OPEN) {
		ui->shutterOpenBox->setChecked(true);
	} else {
		ui->shutterOpenBox->setChecked(false);
	}
	ui->shutterOpenBox->blockSignals(false);
}

/**
 * \brief Set new Shutter settings
 *
 * This method changes the shutter setting of the Exposure and then
 * sends the exposureChanged() signal
 */
void	ccdcontrollerwidget::setShutter(Shutter::state s) {
	if (s == _exposure.shutter()) {
		return;
	}
	displayShutter(s);
	emit exposureChanged(_exposure);
}

/**
 * \brief internal Slot activate when a gui element changes
 */
void	ccdcontrollerwidget::guiChanged() {
	if (sender() == ui->binningSelectionBox) {
		_exposure.mode(getBinning(
			ui->binningSelectionBox->currentIndex()));
	}
	if (sender() == ui->exposureSpinBox) {
		_exposure.exposuretime(ui->exposureSpinBox->value());
	}
	if (sender() == ui->purposeBox) {
		_exposure.purpose(getPurpose(ui->purposeBox->currentIndex()));
	}
	if (sender() == ui->shutterOpenBox) {
		_exposure.shutter(ui->shutterOpenBox->isChecked()
			? Shutter::OPEN : Shutter::CLOSED);
	}
	emit exposureChanged(_exposure);
}


/**
 * \brief Slot to handle a new image
 */
void	ccdcontrollerwidget::setImage(ImagePtr image) {
	_image = image;
	emit imageReceived();
}

/**
 * \brief Slot to handle a change of the selected CCD
 */
void	ccdcontrollerwidget::ccdChanged(int index) {
	_ccd = _instrument.ccd(index);
        setupCcd();
}

} // namespace snowgui
