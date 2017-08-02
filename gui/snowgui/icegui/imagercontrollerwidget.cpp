/*
 * imagercontrollerwidget.cpp --  ccd controller implementation
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "imagercontrollerwidget.h"
#include "ui_imagercontrollerwidget.h"
#include <IceConversions.h>
#include <algorithm>
#include <QTimer>
#include <AstroIO.h>
#include <AstroImageops.h>
#include <QMessageBox>
#include "darkwidget.h"

using namespace astro::image;
using namespace astro::io;
using namespace astro::camera;

namespace snowgui {

/**
 * \brief Constructor for the CCD controller
 */
imagercontrollerwidget::imagercontrollerwidget(QWidget *parent) :
	InstrumentWidget(parent), ui(new Ui::imagercontrollerwidget) {
	// setup user interface components
	ui->setupUi(this);

	// install all internal connections
	connect(ui->exposureSpinBox, SIGNAL(valueChanged(double)),
		this, SLOT(guiChanged()));
	connect(ui->binningSelectionBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(guiChanged()));

	connect(ui->captureButton, SIGNAL(clicked()),
		this, SLOT(captureClicked()));
	connect(ui->darkButton, SIGNAL(clicked()),
		this, SLOT(darkClicked()));
	connect(ui->flatButton, SIGNAL(clicked()),
		this, SLOT(flatClicked()));

	connect(ui->frameFullButton, SIGNAL(clicked()),
		this, SLOT(guiChanged()));

	connect(ui->darkBox, SIGNAL(checked(bool)),
		this, SLOT(toggleDark(bool)));
	connect(ui->flatBox, SIGNAL(checked(bool)),
		this, SLOT(toggleFlat(bool)));
	connect(ui->interpolateBox, SIGNAL(checked(bool)),
		this, SLOT(toggleInterpolate(bool)));

	// setup and connect the timer
	connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	statusTimer.setInterval(100);
	ourexposure = false;
	previousstate = snowstar::GuiderUNCONFIGURED;
	_nosubframe = false;
	_nobuttons = false;

	// make sure the widget cannot be used unless a CCD is configured
	ui->ccdInfo->setEnabled(false);
	ui->frameWidget->setEnabled(false);
	ui->buttonArea->setEnabled(false);
}

/**
 * \brief Common instrument setup
 *
 * This method is called to add instrument information. The default
 * constructor called by the Designer generated code cannot include
 * this information, so we supply it later..
 */
void	imagercontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin imagercontrollerwidget::instrumentSetup()");
	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// read information about CCDs available on this instrument, and 
	// remember the first ccd you can find
	_guider = instrument.guider(0, 0, 0);

	// add additional information about this ccd
	setupCcd();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end imagercontrollerwidget::instrumentSetup()");
}

/**
 * \brief Destroy the CCD controller
 */
imagercontrollerwidget::~imagercontrollerwidget() {
	statusTimer.stop();
	delete ui;
}

/**
 * \brief Read information from the ccd and show it
 */
void	imagercontrollerwidget::setupCcd() {
	// we set the previous state to idle, but if that is not correct, then
	// then the first status update will fix it.
	previousstate = snowstar::GuiderUNCONFIGURED;
	ui->captureButton->setEnabled(true);
	ui->darkButton->setEnabled(true);
	ui->flatButton->setEnabled(true);

	// make sure no signals are sent while setting up the CCD
	ui->binningSelectionBox->setEnabled(false);
	ui->binningSelectionBox->blockSignals(true);

	// remove all entry from the binning mode combobox
	while (ui->binningSelectionBox->count() > 0) {
		ui->binningSelectionBox->removeItem(0);
	}

	// propagate the information from the ccdinfo structure
	if (_guider) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "get info");
		_ccdinfo = _guider->getCcd()->getInfo();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got info");

		// set name
		ui->imagerInfoLabel->setText(QString(_ccdinfo.name.c_str()));

		// get binning info
		QComboBox	*binBox = ui->binningSelectionBox;
		CcdInfo	ccdinfo(snowstar::convert(_ccdinfo));
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

		// start the timer
		statusTimer.start();
	}

	// now reenable signals
	ui->binningSelectionBox->blockSignals(false);
	ui->binningSelectionBox->setEnabled(true);

	// enable everything
	ui->ccdInfo->setEnabled(true);
	ui->frameWidget->setEnabled(true);
	ui->buttonArea->setEnabled(true);
}

/**
 * \brief Display the settings in the argument exposure 
 *
 * This method does not send any signals
 */
void	imagercontrollerwidget::displayExposure(Exposure e) {
	displayBinning(e.mode());
	displayExposureTime(e.exposuretime());
}

/**
 * \brief This is the slot to call when setting an exposure from the outside
 *
 * This method ensures that all the GUI elements are updated, and then sends 
 * the exposureChanged() signal to whatever widget is connected to it.
 */
void	imagercontrollerwidget::setExposure(Exposure e) {
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
void	imagercontrollerwidget::displayFrame(ImageRectangle r) {
	// is the rectangle contained in the ccd
	if (!snowstar::convert(_ccdinfo).size().bounds(r)) {
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
void	imagercontrollerwidget::setFrame(ImageRectangle r) {
	if (_exposure.frame() == r) {
		return;
	}
	displayFrame(r);
	emit exposureChanged(_exposure);
}

/**
 * \brief set the frame
 *
 * This method converts the rectangle to CCD coordinates. Note that
 * only this controller knows about the binning mode, so it has to compute
 * unbinned coordinates.
 */
void	imagercontrollerwidget::setSubframe(ImageRectangle r) {
	ImagePoint	origin = r.origin() + _exposure.frame().origin();
	ImageRectangle	newrectangle
		= ImageRectangle(origin, r.size()) * _exposure.mode();
	setFrame(newrectangle);
}

/**
 * \brief Display the new binning mode
 *
 * This method does not send signals
 */
void	imagercontrollerwidget::displayBinning(Binning b) {
	// is binning mode supported by this camera?
	if (!snowstar::convert(_ccdinfo).modes().permits(b)) {
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
 * \brief private mtethod to get the binning mode from the selected item index
 */
Binning	imagercontrollerwidget::getBinning(int index) {
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
void	imagercontrollerwidget::displayBinning(int index) {
	displayBinning(getBinning(index));
}

/**
 * \brief Set the binning mode
 *
 * This method displays the new binning mode (if it is acceptable, i.e.
 * supported by the CCD in use), and then sends the exposureChanged signal.
 */
void	imagercontrollerwidget::setBinning(Binning b) {
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
void	imagercontrollerwidget::displayExposureTime(double t) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new exposure time: %.3f", t);
	_exposure.exposuretime(t);
	ui->exposureSpinBox->blockSignals(true);
	ui->exposureSpinBox->setValue(t);
	ui->exposureSpinBox->blockSignals(false);
}

/**
 * \brief Set the exposure time
 */
void	imagercontrollerwidget::setExposureTime(double t) {
	displayExposureTime(t);
	emit exposureChanged(_exposure);
}

/**
 * \brief internal Slot activate when a gui element changes
 */
void	imagercontrollerwidget::guiChanged() {
	if (sender() == ui->binningSelectionBox) {
		displayBinning(getBinning(
			ui->binningSelectionBox->currentIndex()));
	}
	if (sender() == ui->exposureSpinBox) {
		displayExposureTime(ui->exposureSpinBox->value());
	}
	if (sender() == ui->frameFullButton) {
		displayFrame(ImageRectangle(snowstar::convert(_ccdinfo).size()));
	}
	emit exposureChanged(_exposure);
}


/**
 * \brief Slot to handle a new image
 */
void	imagercontrollerwidget::setImage(ImagePtr image) {
	_image = image;
	emit imageReceived(_image);
}

/**
 * \brief Slot to handle click on the "Capture" button
 */
void	imagercontrollerwidget::captureClicked() {
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"start exposure with time=%.3f",
			_exposure.exposuretime());
		try {
			_guider->startImaging(snowstar::convert(_exposure));
		} catch (const std::exception& x) {
			ccdFailed(x);
			return;
		}
		ourexposure = true;
		ui->captureButton->setEnabled(false);
		ui->darkButton->setEnabled(false);
		ui->flatButton->setEnabled(false);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot start Imaging");
	}
}

/**
 * \brief Slot tohandle click on the "Dark" button
 */
void	imagercontrollerwidget::darkClicked() {
	if (!_guider) {
		return;
	}
	darkwidget	*dw = new darkwidget(NULL);
	dw->guider(_guider);

	std::ostringstream	out;
	out << "dark image for ";
	astro::guiding::GuiderDescriptor	gd = convert(_guider->getDescriptor());
	out << gd.toString();
	
	dw->setWindowTitle(QString(out.str().c_str()));
	dw->exposuretime(_exposure.exposuretime());
	dw->show();
}

/**
 * \brief Slot to handle click on the "Flat" button
 */
void	imagercontrollerwidget::flatClicked() {
	try {
		_guider->startFlatAcquire(_exposure.exposuretime(), 10);
	} catch (const snowstar::BadState& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "bad state: %s", x.what());
	} catch (const std::exception& x) {
		ccdFailed(x);
		return;
	}
}

/**
 * \brief Slot to handle new images
 *
 * This method retrieves an image from the remote server and then emits
 * the imageReceived signal
 */
void	imagercontrollerwidget::retrieveImage() {
	// it may happen that some other program initiated the exposure,
	// so we check whether this is our exposure. If not, we give up
	// at this point
	if (!ourexposure) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not our exposure, giving up");
		return;
	}
	ourexposure = false;
	try {
		_imageproxy = _guider->getImage();
		if (!_imageproxy->hasMeta("INSTRUME")) {
			snowstar::Metavalue	v;
			v.keyword = "INSTRUME";
			v.value = instrumentname();
			_imageproxy->setMetavalue(v);
		}
		ImagePtr	image = snowstar::convert(_imageproxy);
		_image = image;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image frame: %s",
			image->getFrame().toString().c_str());
		_imageexposure = snowstar::convert(_guider->getCcd()->getExposure());
		_imageproxy->remove();

		// if the image size does not match the size requested, get the
		// subimage
		if (_image->getFrame() != _exposure.frame()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cutting image to %s",
				_exposure.frame().toString().c_str());
			_image = astro::image::ops::cut(_image, _exposure.frame());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image dimensions now %s",
			_image->getFrame().toString().c_str());

		debug(LOG_DEBUG, DEBUG_LOG, 0, "image received, emit signal");
		emit imageReceived(_image);
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("cannot retrieve "
			"image: exception %s, cause=%s",
			astro::demangle(typeid(x).name()).c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		ccdFailed(x);
	}
}

/**
 * \brief Status update slot
 *
 * This slot is called by the 
 */
void	imagercontrollerwidget::statusUpdate() {
	if (!_guider) {
		return;
	}
	snowstar::GuiderState	newstate;
	try {
		newstate = _guider->getState();
	} catch (const std::exception& x) {
		ccdFailed(x);
		return;
	}
	if (newstate == previousstate) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new state: %d (%d)", newstate,
		previousstate);
	if (previousstate == snowstar::GuiderIMAGING) {
		snowstar::ImagePrx	image = _guider->getImage();
		astro::image::ImagePtr	imageptr = snowstar::convert(image);
		emit imageReceived(imageptr);
	}
	switch (newstate) {
	case snowstar::GuiderIDLE:
	case snowstar::GuiderUNCONFIGURED:
	case snowstar::GuiderCALIBRATED:
		ui->captureButton->setEnabled(true);
		ui->darkButton->setEnabled(true);
		ui->flatButton->setEnabled(true);
		break;
	case snowstar::GuiderCALIBRATING:
	case snowstar::GuiderGUIDING:
	case snowstar::GuiderDARKACQUIRE:
	case snowstar::GuiderFLATACQUIRE:
	case snowstar::GuiderIMAGING:
		ui->captureButton->setEnabled(false);
		ui->darkButton->setEnabled(false);
		ui->flatButton->setEnabled(false);
		break;
	}
	previousstate = newstate;

	// find out whether the guider has dark/flat images
	bool	hasdark = _guider->hasDark();
	ui->darkBox->setEnabled(hasdark);
	ui->interpolateBox->setEnabled(hasdark);
	bool	hasflat = _guider->hasFlat();
	ui->flatBox->setEnabled(hasflat);

	ui->darkBox->blockSignals(true);
	ui->darkBox->setChecked(_guider->useDark());
	ui->darkBox->blockSignals(false);

	ui->interpolateBox->blockSignals(true);
	ui->interpolateBox->setChecked(_guider->interpolate());
	ui->interpolateBox->blockSignals(false);

	ui->flatBox->blockSignals(true);
	ui->flatBox->setChecked(_guider->useFlat());
	ui->flatBox->blockSignals(false);
}

void	imagercontrollerwidget::hideSubframe(bool sf) {
	_nosubframe = sf;
	ui->frameWidget->setHidden(_nosubframe);
}

void	imagercontrollerwidget::hideButtons(bool b) {
	_nobuttons = b;
	ui->buttonArea->setHidden(_nobuttons);
}

void	imagercontrollerwidget::ccdFailed(const std::exception& x) {
	_guider = NULL;
	ui->ccdInfo->setEnabled(false);
	ui->frameWidget->setEnabled(false);
	ui->buttonArea->setEnabled(false);
	QMessageBox	message;
	message.setText(QString("Guider failed"));
	std::ostringstream	out;
	out << "Communication with the Guider Imager '";
	out << ui->imagerInfoLabel->text().toLatin1().data();
	out << "' failed." << std::endl;
	out << "The reason for the failure was: " << x.what() << std::endl;
	out << "The Imager has been disabled and can no longer be used.";
	message.setInformativeText(QString(out.str().c_str()));
	message.exec();
}

void	imagercontrollerwidget::toggleDark(bool t) {
	try {
		_guider->setUseDark(t);
	} catch (...) {
	}
}

void	imagercontrollerwidget::toggleFlat(bool t) {
	try {
		_guider->setUseFlat(t);
	} catch (...) {
	}
}

void	imagercontrollerwidget::toggleInterpolate(bool t) {
	try {
		_guider->setInterpolate(t);
	} catch (...) {
	}
}

} // namespace snowgui
