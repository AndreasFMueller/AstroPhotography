/*
 * focusingcontrollerwidget.cpp -- focusing controller implementation
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "focusingcontrollerwidget.h"
#include "ui_focusingcontrollerwidget.h"

#include <InstrumentWidget.h>
#include <focusing.h>
#include <IceConversions.h>
#include <CommunicatorSingleton.h>

namespace snowgui {

/**
 * \brief Construct a new focusingcontrollerwidget
 */
focusingcontrollerwidget::focusingcontrollerwidget(QWidget *parent) :
	InstrumentWidget(parent), ui(new Ui::focusingcontrollerwidget) {
	ui->setupUi(this);

	_previousstate = snowstar::FocusIDLE;
	_steps = ui->stepsSpinBox->value();
	_stepsize = ui->stepsizeSpinBox->value();
	_center = ui->centerSpinBox->value();

	// set up connections
	connect(ui->startButton, SIGNAL(clicked()),
		this, SLOT(startClicked()));
	connect(ui->stepsSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(stepsChanged(int)));
	connect(ui->stepsizeSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(stepsizeChanged(int)));
	connect(ui->centerSpinBox, SIGNAL(valueChanged(int)),
		this, SLOT(centerChanged(int)));

	connect(&_timer, SIGNAL(timeout()),
		this, SLOT(statusUpdate()));

	// initialize the timer
	_timer.setInterval(1000);
}

/**
 * \brief Destroy the focusingcontrollerwidget
 */
focusingcontrollerwidget::~focusingcontrollerwidget() {
	delete ui;
}

/**
 * \brief Instrument setup
 */
void	focusingcontrollerwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	// parent initializations
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// create the guiderfactory proxy
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
	astro::ServerName       servername(serviceobject.name());
	Ice::ObjectPrx  fbase = ic->stringToProxy(
					servername.connect("FocusingFactory"));
	_focusingfactory = snowstar::FocusingFactoryPrx::checkedCast(fbase);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got the focusers factory");

	// get the first imager ccd and the first focuser from a remote
	// instrument
	if (_instrument.has(snowstar::InstrumentFocuser, 0)) {
		_focuser = instrument.focuser(0);
		_focusername = _focuser->getName();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "focuser name: %s",
			_focusername.c_str());
	}
	if (_instrument.has(snowstar::InstrumentCCD, 0)) {
		_ccd = instrument.ccd(0);
		_ccdname = _ccd->getName();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd name: %s",
			_ccdname.c_str());
	}

	// get the focusing object
	_focusing = _focusingfactory->get(_ccdname, _focusername);
}

/**
 * \brief Main thread initializations
 */
void	focusingcontrollerwidget::setupComplete() {
	// set the evaluation methods for the menu
	ui->evaluationBox->blockSignals(true);
	snowstar::FocusMethods	methods = _focusingfactory->getMethods();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d methods", methods.size());
	std::for_each(methods.begin(), methods.end(),
		[&](const std::string& method) {
			ui->evaluationBox->addItem(QString(method.c_str()));
		}
	);
	ui->evaluationBox->blockSignals(false);

	// set the solvers for the menu
	ui->solverBox->blockSignals(true);
	snowstar::FocusSolvers	solvers = _focusingfactory->getSolvers();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d solvers", solvers.size());
	std::for_each(solvers.begin(), solvers.end(),
		[&](const std::string& solver) {
			ui->solverBox->addItem(QString(solver.c_str()));
		}
	);
	ui->solverBox->blockSignals(false);

	// get the current focuser
	ui->centerSpinBox->setValue(_focuser->current());

	// start the timer
	_timer.start();
}

/**
 * \brief Slot executed when start is clicked
 */
void	focusingcontrollerwidget::startClicked() {
	if (!_focusing) {
		return;
	}

	switch (_focusing->status()) {
	case snowstar::FocusIDLE:
	case snowstar::FocusFOCUSED:
	case snowstar::FocusFAILED:
		start();
		break;
	case snowstar::FocusMOVING:
	case snowstar::FocusMEASURING:
	case snowstar::FocusMEASURED:
		stop();
		break;
	}
	statusUpdate();
}

/**
 * \brief Stop the focusing
 */
void	focusingcontrollerwidget::stop() {
	_focusing->cancel();
}

/**
 * \brief Start the focusing
 */
void	focusingcontrollerwidget::start() {
	int	steps = _steps;
	if (steps % 2) {
		steps++;
	}
	int	start = _center - _stepsize * (steps / 2);
	int	end = _center + _stepsize * (steps / 2);

	_focusing->setMethod(std::string(
		ui->evaluationBox->currentText().toLatin1().data()));
	_focusing->setSolver(std::string(
		ui->solverBox->currentText().toLatin1().data()));
	_focusing->setExposure(snowstar::convert(_exposure));
	_focusing->setSteps(steps);
	_focusing->start(start, end);
}

/**
 * \brief Slot executed when the exposure in the ccdcontrollerwidget changes
 *
 * \param exposure
 */
void	focusingcontrollerwidget::exposureChanged(
		astro::camera::Exposure exposure) {
	_exposure = exposure;
}

void	focusingcontrollerwidget::stepsizeChanged(int s) {
	_stepsize = s;
}

void	focusingcontrollerwidget::stepsChanged(int s) {
	_steps = s;
}

void	focusingcontrollerwidget::centerChanged(int s) {
	_center = s;
}

void	focusingcontrollerwidget::statusUpdate() {
	if (!_focusing) {
		return;
	}
	snowstar::FocusState	newstate = _focusing->status();
	if (newstate == _previousstate) {
		return;
	}
	switch (_focusing->status()) {
	case snowstar::FocusIDLE:
	case snowstar::FocusFOCUSED:
	case snowstar::FocusFAILED:
		ui->evaluationBox->setEnabled(true);
		ui->solverBox->setEnabled(true);
		ui->stepsSpinBox->setEnabled(true);
		ui->stepsizeSpinBox->setEnabled(true);
		ui->centerSpinBox->setEnabled(true);
		ui->startButton->setText(QString("Start"));
		break;
	case snowstar::FocusMOVING:
	case snowstar::FocusMEASURING:
	case snowstar::FocusMEASURED:
		ui->evaluationBox->setEnabled(false);
		ui->solverBox->setEnabled(false);
		ui->stepsSpinBox->setEnabled(false);
		ui->stepsizeSpinBox->setEnabled(false);
		ui->centerSpinBox->setEnabled(false);
		ui->startButton->setText(QString("Stop"));
		break;
	}
	_previousstate = newstate;
}

} // namespace snowgui
