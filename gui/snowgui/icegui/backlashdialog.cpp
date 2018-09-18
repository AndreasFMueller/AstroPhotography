/*
 * backlashdialog.cpp -- Dialog to control backlash characterization
 *
 * (c) 2017 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "backlashdialog.h"
#include "ui_backlashdialog.h"
#include <AstroDebug.h>
#include <IceConversions.h>
#include <CommunicatorSingleton.h>
#include "BacklashMonitor.h"

namespace snowgui {

/**
 * \brief Construct a new BacklashDialog
 */
BacklashDialog::BacklashDialog(QWidget *parent) :
	QDialog(parent), ui(new Ui::BacklashDialog) {
	ui->setupUi(this);

	// general initialization
	_monitor = NULL;
	_previousstate = snowstar::GuiderUNCONFIGURED;
	_data.result.x = 0;
	_data.result.y = 0;

	ui->dataWidget->addChannel(QColor(0, 255, 0));
	ui->dataWidget->addChannel(QColor(0, 0, 255));
	ui->dataWidget->addChannel(QColor(255, 0, 0));
	ui->dataWidget->drawstddev(false);

	// initialize the timer
	statusTimer.setInterval(100);
	connect(&statusTimer, SIGNAL(timeout()), this, SLOT(statusUpdate()));

	// connect widgets
	connect(ui->startButton, SIGNAL(clicked()), this, SLOT(startClicked()));
	connect(ui->lastpointsSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(lastpointsChanged(int)));
}

/**
 * \brief Destroy the backlash dialog
 */
BacklashDialog::~BacklashDialog() {
	delete ui;
}

/**
 * \brief Install a guider into the backlash dialog class
 */
void	BacklashDialog::guider(snowstar::GuiderPrx guider) {
	statusTimer.stop();
	// unregister the present monitor
	if (NULL != _monitor) {
		disconnect(_monitor, SIGNAL(stopSignal()), 0, 0);
		disconnect(_monitor,
			SIGNAL(updatePointSignal(snowstar::BacklashPoint)),
			0, 0);
		disconnect(_monitor,
			SIGNAL(updateResultSignal(snowstar::BacklashResult)),
			0, 0);
		if (_guider) {
			_guider->unregisterBacklashMonitor(_monitoridentity);
		}
	}

	_monitor = new BacklashMonitor(this);
	_guider = guider;
	if (!_guider) {
		return;
	}

	_previousstate = snowstar::GuiderUNCONFIGURED;
	statusUpdate();
	statusTimer.start();

	try {
		_data = _guider->getBacklashData();
		showResult();
		reloadPoints();
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get backlash data");
	}

	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"registering the backlash monitor");
		snowstar::CommunicatorSingleton::connect(guider);
		Ice::ObjectPtr	_monitorptr(_monitor);
		_monitoridentity = snowstar::CommunicatorSingleton::add(
					_monitorptr);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "identity: %s",
			_monitoridentity.name.c_str());
		_guider->registerBacklashMonitor(_monitoridentity);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register: %s",
			x.what());
		_monitor = NULL;
		return;
	}

	// connect monitor to slots of this widget
	connect(_monitor, SIGNAL(stopSignal()),
		this, SLOT(stopSignaled()));
	connect(_monitor, SIGNAL(updatePointSignal(snowstar::BacklashPoint)),
		this, SLOT(updatePointSignaled(snowstar::BacklashPoint)));
	connect(_monitor, SIGNAL(updateResultSignal(snowstar::BacklashResult)),
		this, SLOT(updateResultSignaled(snowstar::BacklashResult)));
}

/**
 * \brief Check for status changes
 */
void	BacklashDialog::statusUpdate() {
	if (!_guider) {
		return;
	}
	snowstar::GuiderState	newstate;
	try {
		newstate = _guider->getState();
	} catch (const std::exception& x) {
		// XXX handle exception
		return;
	}
	if (newstate == _previousstate) {
		return;
	}
	switch (newstate) {
	case snowstar::GuiderIDLE:
	case snowstar::GuiderUNCONFIGURED:
	case snowstar::GuiderCALIBRATED:
		ui->startButton->setText("Start");
		ui->intervalSpinBox->setEnabled(true);
		break;
	case snowstar::GuiderCALIBRATING:
	case snowstar::GuiderGUIDING:
	case snowstar::GuiderDARKACQUIRE:
	case snowstar::GuiderFLATACQUIRE:
	case snowstar::GuiderIMAGING:
		ui->intervalSpinBox->setEnabled(true);
		ui->startButton->setText("Stop");
		break;
	case snowstar::GuiderBACKLASH:
		ui->intervalSpinBox->setEnabled(false);
		ui->startButton->setText("Stop");
		break;
	}
}

/**
 * \brief Slot clicked when backlash characterization is supposed to start
 */
void	BacklashDialog::startClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "startClicked()");
	if (!_guider) {
		return;
	}
	switch (_guider->getState()) {
	case snowstar::GuiderUNCONFIGURED:
	case snowstar::GuiderIDLE:
	case snowstar::GuiderCALIBRATED:
		try {
			int	interval = ui->intervalSpinBox->value();
			_guider->startBacklash(interval, _direction);
			_data.result.x = 0;
			_data.result.y = 0;
			_data.result.forward = 0;
			_data.result.backward = 0;
			_data.result.f = 0;
			_data.result.b = 0;
			_data.result.lateral = 0;
			_data.result.longitudinal = 0;
			showResult();
			_data.points.clear();
			reloadPoints();
			windowTitle();
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot start backlash: %s",
				x.what());
		}
		break;
	case snowstar::GuiderBACKLASH:
		try {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "try to stop backlash");
			_guider->stopBacklash();
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot sotp backlash: %s",
				x.what());
		}
		break;
	default:
		break;
	}
}

/**
 * \brief
 */
void	BacklashDialog::stopSignaled() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop signaled");
}

/**
 * \brief Method to add a point to the result data and to the channeldisplay
 */
void	BacklashDialog::addPoint(const snowstar::BacklashPoint& point) {
	std::vector<double>	values;
	values.push_back(point.xoffset);
	values.push_back(point.yoffset);
	if (hypot(_data.result.x, _data.result.y) > 0.5) {
		values.push_back(point.xoffset * _data.result.x +
			point.yoffset * _data.result.y);
	} else {
		//values.push_back(std::numeric_limits<double>::quiet_NaN());
		values.push_back(0);
	}
	ui->dataWidget->add(point.time, values);
}

/**
 * \brief Method to set the window title
 */
void	BacklashDialog::windowTitle() {
	std::string	title = std::string(
		(_direction == snowstar::BacklashDEC) ? "DEC" : "RA");
	title = title + std::string(" backlash");
	if (_data.points.size() > 0) {
		title = title + astro::stringprintf(": %d points",
			_data.points.size());
	}
	this->setWindowTitle(title.c_str());
}

/**
 * \brief Slot called whtne the upatePointSignal fires
 */
void	BacklashDialog::updatePointSignaled(snowstar::BacklashPoint point) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new point: %s",
		convert(point).toString().c_str());
	_data.points.push_back(point);
	addPoint(point);
	windowTitle();
}

/**
 * \brief Reload the points
 */
void	BacklashDialog::reloadPoints() {
	ui->dataWidget->clearData();
	std::vector<snowstar::BacklashPoint>::const_iterator	i;
	for (i = _data.points.begin(); i != _data.points.end(); i++) {
		addPoint(*i);
	}
	ui->dataWidget->repaint();
}

/**
 * \brief Update the result
 *
 * This also means that the points have to be reloaded, because the eigenvector
 * of the covariance matrix may have changed.
 */
void	BacklashDialog::updateResultSignaled(snowstar::BacklashResult result) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new result: %s",
		convert(result).toString().c_str());
	_data.result = result;
	showResult();
	reloadPoints();
}

/**
 * \brief Show the numerical data of the result in the form fields
 */
void	BacklashDialog::showResult() {
	if (hypot(_data.result.x, _data.result.y) < 0.5) {
		ui->directionField->setText("");
		ui->directionField2->setText("");
		ui->directionFieldAngle->setText("");
		ui->scatterField->setText("");
		ui->scatterFieldLength->setText("");
		ui->movementField->setText("");
		ui->movementField2->setText("");
		ui->backlashField->setText("");
		ui->backlashField2->setText("");
		ui->offsetField->setText("");
		ui->offsetField2->setText("");
		ui->lastpointsField->setText("");
		return;
	}
	ui->directionField->setText(
		astro::stringprintf("%.1f,", _data.result.x).c_str());
	ui->directionField2->setText(
		astro::stringprintf("%.1f", _data.result.y).c_str());
	ui->directionFieldAngle->setText(astro::stringprintf("%.1f°",
		(180 / M_PI) * atan2(_data.result.y, _data.result.x)).c_str());

	ui->scatterField->setText(
		astro::stringprintf("%.1f,", _data.result.longitudinal).c_str());
	ui->scatterField2->setText(
		astro::stringprintf("%.1f", _data.result.lateral).c_str());
	ui->scatterFieldLength->setText(astro::stringprintf("%.1f",
		hypot(_data.result.longitudinal, _data.result.lateral)).c_str());

	ui->movementField->setText(
		astro::stringprintf("%.1f,", _data.result.forward).c_str());
	ui->movementField2->setText(
		astro::stringprintf("%.1f", _data.result.backward).c_str());

	ui->backlashField->setText(astro::stringprintf("%.1f,",
		_data.result.forward - _data.result.f).c_str());
	ui->backlashField2->setText(astro::stringprintf("%.1f",
		_data.result.backward - _data.result.b).c_str());

	ui->offsetField->setText(
		astro::stringprintf("%.1f [px],", _data.result.offset).c_str());
	ui->offsetField2->setText(
		astro::stringprintf("%.3f [px/s]", _data.result.drift).c_str());

	if (0 == _data.result.lastpoints) {
		ui->lastpointsField->setText("all");
	} else {
		ui->lastpointsField->setText(astro::stringprintf("%d",
			_data.result.lastpoints).c_str());
	}
	ui->lastpointsSpinBox->blockSignals(true);
	ui->lastpointsSpinBox->setValue(_data.result.lastpoints);
	ui->lastpointsSpinBox->blockSignals(false);
}

/**
 * \brief slot called when the number of points to consider changes
 */
void	BacklashDialog::lastpointsChanged(int lastpoints) {
	if (!_guider) {
		return;
	}
	try {
		_guider->setLastPoints(lastpoints);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set last points: %d",
			lastpoints);
	}
}

/**
 * \brief Set the backlash assessment direction
 */
void	BacklashDialog::direction(snowstar::BacklashDirection d) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting backlash direction to %s",
		(d == snowstar::BacklashDEC) ? "DEC" : "RA");
	_direction = d;
}

snowstar::BacklashDirection	BacklashDialog::direction() const {
	return _direction;
}

} // namespace snowgui
