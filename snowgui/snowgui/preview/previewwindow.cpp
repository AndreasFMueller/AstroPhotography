/*
 * previewwindow.cpp -- implementation of the preview application
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "previewwindow.h"
#include "ui_previewwindow.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <camera.h>
#include <QTimer>
#include <QScrollBar>
#include "PreviewImageSink.h"
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include "Image2Pixmap.h"

using namespace astro::discover;

namespace snowgui {

PreviewWindow::PreviewWindow(QWidget *parent)
	: QWidget(parent), ui(new Ui::PreviewWindow) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting PreviewWindow");
	ui->setupUi(this);
}

void	PreviewWindow::instrumentSetup(ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	_servicekey = serviceobject;
	_instrument = instrument;

	// get the instrument name into the title
	std::string	title
		= astro::stringprintf("Preview instrument %s @ %s",
		_instrument.name().c_str(), serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview starting on instrument %s",
		_instrument.name().c_str());

	// display the gain/brightness settings
	displayGainSettings();
	displayBrightnessSettings();
	displayScaleSettings();

	// read component names and initialize the combo boxes
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
		snowstar::CcdPrx	ccd = _instrument.guiderccd(index);
		if (!_ccd) {
			_ccd = ccd;
		}
		ui->ccdSelectionBox->addItem(QString(ccd->getName().c_str()));
		index++;
	}

	// cooler
	index = 0;
	while (_instrument.has(snowstar::InstrumentCooler, index)) {
		snowstar::CoolerPrx	cooler
			= _instrument.cooler(index);
		if (!_cooler) {
			_cooler = cooler;
		}
		ui->coolerSelectionBox->addItem(
			QString(cooler->getName().c_str()));
		index++;
	}

	// filterwheel
	index = 0;
	while (_instrument.has(snowstar::InstrumentFilterWheel, index)) {
		snowstar::FilterWheelPrx	filterwheel
			= _instrument.filterwheel(index);
		if (!_filterwheel) {
			_filterwheel = filterwheel;
		}
		ui->filterwheelSelectionBox->addItem(
			QString(filterwheel->getName().c_str()));
		index++;
	}

	// focuser
	index = 0;
	while (_instrument.has(snowstar::InstrumentFocuser, index)) {
		snowstar::FocuserPrx	focuser
			= _instrument.focuser(index);
		if (!_focuser) {
			_focuser = focuser;
		}
		ui->focuserSelectionBox->addItem(
			QString(focuser->getName().c_str()));
		index++;
	}

	// guiderport
	index = 0;
	while (_instrument.has(snowstar::InstrumentGuiderPort, index)) {
		snowstar::GuiderPortPrx	guiderport
			= _instrument.guiderport(index);
		if (!_guiderport) {
			_guiderport = guiderport;
		}
		ui->guiderportSelectionBox->addItem(
			QString(guiderport->getName().c_str()));
		index++;
	}

	// get information about the devices
	setupCcd();
	setupCooler();
	setupFilterwheel();
	setupFocuser();
	setupGuiderport();

	// connect signals
	connect(this, SIGNAL(imageUpdated()), this,
                SLOT(processImage()), Qt::QueuedConnection);

	// start the timer
	statusTimer = new QTimer();
	connect(statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));
	statusTimer->setInterval(1000);
	statusTimer->start();
}

PreviewWindow::~PreviewWindow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy PreviewWindow");
	statusTimer->stop();
	delete statusTimer;
	delete ui;
}

void	PreviewWindow::setImage(astro::image::ImagePtr image) {
	_image = image;
	emit imageUpdated();
}

void	PreviewWindow::processImage() {
	// don't do anything if there is no image
	if (!_image) {
		return;
	}
	astro::image::ImageSize	size = _image->size();

	// remember the current position of the scroll area
	int	hpos = ui->scrollArea->horizontalScrollBar()->value();
	int	vpos = ui->scrollArea->verticalScrollBar()->value();
	QSize	previoussize = ui->scrollArea->widget()->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"hpos = %d, vpos = %d, previous size=%d,%d",
		hpos, vpos, previoussize.width(), previoussize.height());

	// set the size of imageLabel
	QLabel	*imageLabel = new QLabel;

	QPixmap	*pixmap = image2pixmap(_image);
	if (NULL != pixmap) {
		imageLabel->setPixmap(*pixmap);
	}
	imageLabel->setFixedSize(pixmap->width(), pixmap->height());
	imageLabel->setMinimumSize(pixmap->width(), pixmap->height());

	ui->scrollArea->setWidget(imageLabel);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new position: %d/%d", hpos, vpos);
	QSize	newsize = pixmap->size();
	hpos = newsize.width() * hpos / previoussize.width();
	vpos = newsize.height() * vpos / previoussize.height();
	ui->scrollArea->horizontalScrollBar()->setValue(hpos);
	ui->scrollArea->verticalScrollBar()->setValue(vpos);
	ui->scrollArea->show();

	// update the histogram
	QPixmap	*histogram = image2pixmap.histogram(ui->histogramLabel->width(),
		ui->histogramLabel->height());
	if (NULL != histogram) {
		ui->histogramLabel->setPixmap(*histogram);
	}

	//delete pixmap;
}

void	PreviewWindow::displayGainSettings() {
	double	v = ui->gainSlider->value();
	double	g = pow(2, (double)v / 32.);
	std::string	gainstring;
	if (g >= 1) {
		gainstring = astro::stringprintf("%.1f", g);
	} else {
		gainstring = astro::stringprintf("1/%.1f", 1/g);
	}
	ui->gainField->setText(QString(gainstring.c_str()));
}

void	PreviewWindow::displayBrightnessSettings() {
	double	b = ui->brightnessSlider->value();
	std::string	brightnessstring = astro::stringprintf("%.0f", b);
	ui->brightnessField->setText(QString(brightnessstring.c_str()));
}

void	PreviewWindow::displayScaleSettings() {
	double	s = 100 * pow(2, ui->scaleSlider->value());
	std::string	scalestring = astro::stringprintf("%.0f%%", s);
	ui->scaleField->setText(QString(scalestring.c_str()));
}

void	PreviewWindow::imageSettingsChanged() {
	bool	imagehaschanged = false;
	if (sender() == ui->gainSlider) {
		displayGainSettings();
		double	v = ui->gainSlider->value();
		double	g = pow(2, (double)v / 32.);
		image2pixmap.gain(g);
		imagehaschanged = true;
	}
	if (sender() == ui->brightnessSlider) {
		double	b = ui->brightnessSlider->value();
		image2pixmap.brightness(b);
		displayBrightnessSettings();
		imagehaschanged = true;
	}
	if (sender() == ui->scaleSlider) {
		displayScaleSettings();
	}
	if (sender() == ui->logarithmicBox) {
		image2pixmap.logarithmic(ui->logarithmicBox->isChecked());
		imagehaschanged = true;
	}
	if (sender() == ui->scaleSlider) {
		image2pixmap.scale(ui->scaleSlider->value());
		imagehaschanged = true;
	}
	if (imagehaschanged) {
		processImage();
	}
}

astro::camera::Exposure	PreviewWindow::getExposure() {
	astro::camera::Exposure	exposure;
	exposure.exposuretime(ui->exposureSpinBox->value());
	std::string	binning(ui->binningBox->currentText().toLatin1().data());
	exposure.mode(Binning(binning));
	return exposure;
}

void	PreviewWindow::setupCcd() {
	ui->binningBox->setEnabled(false);
	while (ui->binningBox->count() > 0) {
		ui->binningBox->removeItem(0);
	}
	if (_ccd) {
		snowstar::CcdInfo	info = _ccd->getInfo();
		QComboBox	*bBox = ui->binningBox;
		std::for_each(info.binningmodes.begin(),
			info.binningmodes.end(),
			[bBox](const snowstar::BinningMode& mode) {
				std::string	m = astro::stringprintf("%dx%d",
					mode.x, mode.y);
				bBox->addItem(QString(m.c_str()));
			}
		);
		ui->binningBox->setEnabled(true);
	}
}

void	PreviewWindow::setupFilterwheel() {
	ui->filternameBox->setEnabled(false);
	while (ui->filternameBox->count()) {
		ui->filternameBox->removeItem(0);
	}
	if (_filterwheel) {
		int	nfilterwheels = _filterwheel->nFilters();
		for (int i = 0; i < nfilterwheels; i++) {
			std::string	n = _filterwheel->filterName(i);
			std::string	item
				= astro::stringprintf("%d: %s", i+1, n.c_str());
			ui->filternameBox->addItem(QString(item.c_str()));
		}
		try {
			switch (_filterwheel->getState()) {
			case snowstar::FwIDLE:
				ui->filternameBox->setCurrentIndex(
					_filterwheel->currentPosition());
			case snowstar::FwUNKNOWN:
				ui->filterwheelStatus->setEnabled(false);
				ui->filterwheelStatus->setValue(-1);
				break;
			case snowstar::FwMOVING:
				ui->filterwheelStatus->setEnabled(true);
				ui->filterwheelStatus->setValue(0);
				break;
			}
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"cannot get filterwheel status: %s", x.what());
		}
		ui->filternameBox->setEnabled(true);
	}
}

void	PreviewWindow::setupCooler() {
	if (_cooler) {
		float	t = _cooler->getActualTemperature() - 273.15;
		ui->actualTempField->setText(
			QString(astro::stringprintf("%.1f", t).c_str()));
		t = _cooler->getSetTemperature() - 273.15;
		ui->setTempSpinBox->setValue(t);
		ui->coolerOnButton->setEnabled(true);
		if (_cooler->isOn()) {
			ui->coolerOnButton->setText(QString("Off"));
		} else {
			ui->coolerOnButton->setText(QString("On"));
		}
	} else {
		ui->coolerOnButton->setText(QString("On"));
		ui->coolerOnButton->setEnabled(false);
	}
}

void	PreviewWindow::setupFocuser() {
	if (_focuser) {
		ui->focuserCurrentField->setEnabled(true);
		ui->focuserSet->setEnabled(true);
		ui->focuserSet->setSingleStep(10);
		ui->focuserSet->setMinimum(_focuser->min());
		ui->focuserSet->setMaximum(_focuser->max());
		int	s = _focuser->current();
		ui->focuserSet->setValue(s);
		ui->focuserCurrentField->setText(
			QString(astro::stringprintf("%d", s).c_str()));
	} else {
		ui->focuserCurrentField->setText(QString(""));
		ui->focuserCurrentField->setEnabled(false);
		ui->focuserSet->setEnabled(false);
	}
}

void	PreviewWindow::setupGuiderport() {
	if (_guiderport) {
		ui->raplusButton->setEnabled(true);
		ui->raminusButton->setEnabled(true);
		ui->decplusButton->setEnabled(true);
		ui->decminusButton->setEnabled(true);
	} else {
		ui->raplusButton->setEnabled(false);
		ui->raminusButton->setEnabled(false);
		ui->decplusButton->setEnabled(false);
		ui->decminusButton->setEnabled(false);
	}
}

void	PreviewWindow::statusUpdate() {
	if (_cooler) {
		float	t = _cooler->getActualTemperature() - 273.15;
		ui->actualTempField->setText(
			QString(astro::stringprintf("%.1f", t).c_str()));
		t = _cooler->getSetTemperature() - 273.15;
		ui->setTempSpinBox->setValue(t);
		ui->coolerOnButton->setEnabled(true);
	}
	if (_filterwheel) {
		switch (_filterwheel->getState()) {
		case snowstar::FwMOVING:
			ui->filterwheelStatus->setEnabled(true);
			ui->filterwheelStatus->setValue(0);
			ui->filterwheelStatus->setVisible(true);
			break;
		case snowstar::FwIDLE:
			ui->filternameBox->setCurrentIndex(
				_filterwheel->currentPosition());
		default:
			ui->filterwheelStatus->setEnabled(false);
			ui->filterwheelStatus->setValue(-1);
			ui->filterwheelStatus->setVisible(false);
			break;
		}
	}
	if (_focuser) {
		int	s = _focuser->current();
		ui->focuserCurrentField->setText(
			QString(astro::stringprintf("%d", s).c_str()));
	}
	static const char	*white
		= "QButton { background-color : white; }";
	static const char	*transparent
		= "QButton { background-color : transparent; }";
	if (_guiderport) {
		unsigned char	a = _guiderport->active();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "activation: %01x", (int)a);
		if (a & snowstar::DECMINUS) {
			ui->decminusButton->setStyleSheet(white);
		} else {
			ui->decminusButton->setStyleSheet(transparent);
		}
		if (a & snowstar::DECPLUS) {
			ui->decplusButton->setStyleSheet(white);
		} else {
			ui->decplusButton->setStyleSheet(transparent);
		}
		if (a & snowstar::RAMINUS) {
			ui->raminusButton->setStyleSheet(white);
		} else {
			ui->raminusButton->setStyleSheet(transparent);
		}
		if (a & snowstar::RAPLUS) {
			ui->raplusButton->setStyleSheet(white);
		} else {
			ui->raplusButton->setStyleSheet(transparent);
		}
	}
}

void	PreviewWindow::ccdChanged(int ccdindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCCD, index)) {
		if (index == ccdindex) {
			_ccd = _instrument.ccd(index);
		}
		index++;
	}
	index = 0;
	while (_instrument.has(snowstar::InstrumentGuiderCCD, index)) {
		if (index == ccdindex) {
			_ccd = _instrument.ccd(index);
		}
		index++;
	}
	setupCcd();
}

void	PreviewWindow::startStream() {
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();

	// create the image sink
	snowstar::ImageSink	*imagesink = new PreviewImageSink(this);
	_previewimagesink = imagesink;
	Ice::ObjectPtr	callback = imagesink;
	_adapter = snowstar::CallbackAdapterPtr(
			new snowstar::CallbackAdapter(ic));
	Ice::Identity	ident = _adapter->add(callback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setAdapter");
	_ccd->ice_getConnection()->setAdapter(_adapter->adapter());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setAdapter returns, sleeping for 60s");

	// register the imagesink with the 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registering the sink");
	_ccd->registerSink(ident);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registration complete");

	// get the Exposure structure
	astro::camera::Exposure	exposure = getExposure();

	// start the stream
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting the exposure");
	_ccd->startStream(snowstar::convert(exposure));
}

void	PreviewWindow::stopStream() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stopping the stream");
	_ccd->stopStream();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unregistering the sink");
	_ccd->unregisterSink();
	// XXX should remove the sink here
}

void	PreviewWindow::toggleStream() {
	if (!_ccd) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no ccd");
		return;
	}
	if (QString("Start") == ui->startButton->text()) {
		try {
			startStream();
			ui->startButton->setText(QString("Stop"));
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot start stream: %s",
				x.what());
		}
	} else {
		try {
			stopStream();
			ui->startButton->setText(QString("Start"));
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop stream: %s",
				x.what());
		}
	}
}

void	PreviewWindow::exposureChanged() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure changed");
	try {
		_ccd->updateStream(snowstar::convert(getExposure()));
	} catch (...) {
	}
}

void	PreviewWindow::filterwheelChanged(int filterwheelindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentFilterWheel, index)) {
		if (filterwheelindex == index) {
			_filterwheel = _instrument.filterwheel(index);
		}
		index++;
	}
	setupFilterwheel();
}

void	PreviewWindow::filterwheelFilterChanged(int filterindex) {
	if (!_filterwheel) {
		return;
	}
	if (filterindex < 0) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select filter %d", filterindex);
	try {
		_filterwheel->select(filterindex);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot select: %s", x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unkown exception");
	}
}


void	PreviewWindow::coolerChanged(int coolerindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCooler, index)) {
		if (coolerindex == index) {
			_cooler = _instrument.cooler(index);
		}
		index++;
	}
	setupCooler();
}

void	PreviewWindow::coolerTemperatureChanged(double settemperature) {
	if (!_cooler) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set temperature to %.1f",
		settemperature);
	try {
		_cooler->setTemperature(settemperature + 273.15);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot change temperature: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unkown exception");
	}
}

void	PreviewWindow::coolerOnOff() {
	if (!_cooler) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "toggle cooler");
	try {
		if (_cooler->isOn()) {
			_cooler->setOn(false);
			ui->coolerOnButton->setText(QString("On"));
		} else {
			_cooler->setOn(true);
			ui->coolerOnButton->setText(QString("Off"));
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot switch cooler: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unkown exception");
	}
}

void	PreviewWindow::focuserChanged(int focuserindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentFocuser, index)) {
		if (focuserindex == index) {
			_focuser = _instrument.focuser(index);
		}
		index++;
	}
	setupFocuser();
}

void	PreviewWindow::focuserSetChanged(int focusposition) {
	if (!_focuser) {
		return;
	}
	try {
		_focuser->set(focusposition);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot move focuser: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unkown exception");
	}
}

void	PreviewWindow::guiderportChanged(int guiderportindex) {
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentGuiderPort, index)) {
		if (guiderportindex == index) {
			_guiderport = _instrument.guiderport(index);
		}
		index++;
	}
	setupGuiderport();
}

void	PreviewWindow::guiderportActivated() {
	if (!_guiderport) {
		return;
	}
	static const float	defaultactivation = 5;
	try {
		if (sender() == ui->raplusButton) {
			_guiderport->activate(defaultactivation, 0);
		}
		if (sender() == ui->raminusButton) {
			_guiderport->activate(-defaultactivation, 0);
		}
		if (sender() == ui->decplusButton) {
			_guiderport->activate(0, defaultactivation);
		}
		if (sender() == ui->decminusButton) {
			_guiderport->activate(0, -defaultactivation);
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot activate guiderport: %s",
			x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unkown exception");
	}
}

} // namespace snowgui
