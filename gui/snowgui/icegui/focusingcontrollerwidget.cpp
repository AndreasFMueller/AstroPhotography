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
#include <CommonClientTasks.h>

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
	connect(ui->repositoryBox, SIGNAL(currentTextChanged(const QString&)),
		this, SLOT(repositoryChanged(const QString&)));

	connect(&_timer, SIGNAL(timeout()),
		this, SLOT(statusUpdate()));

	// initialize the timer
	_timer.setInterval(1000);

	// create a new callback
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up the callback");
	_callback = new FocusingCallbackI();
	callback = Ice::ObjectPtr(_callback);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "callback installed");

	// connect the callback to the gui
	connect(_callback, SIGNAL(pointReceived(snowstar::FocusPoint)),
		this, SLOT(receivePoint(snowstar::FocusPoint)));
	connect(_callback, SIGNAL(stateReceived(snowstar::FocusState)),
		this, SLOT(receiveState(snowstar::FocusState)));
	connect(_callback,
		SIGNAL(focuselementReceived(snowstar::FocusElement)),
		this,
		SLOT(receiveFocusElement(snowstar::FocusElement)));
}

/**
 * \brief Destroy the focusingcontrollerwidget
 */
focusingcontrollerwidget::~focusingcontrollerwidget() {
	_timer.stop();
	_focusing->unregisterCallback(_ident);
	// we should also remove it from the adapter
	delete ui;
}

/**
 * \brief Instrument setup
 *
 * \param serviceobject
 * \param instrument
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registering the callback");

	// setting up the callback
        _adapter = snowstar::CallbackAdapterPtr(
                        new snowstar::CallbackAdapter(ic));
        _ident = _adapter->add(callback);
        _focusing->ice_getConnection()->setAdapter(_adapter->adapter());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "registering %s", _ident.name.c_str());
	_focusing->registerCallback(_ident);

	// get the repositories proxy
	Ice::ObjectPrx	rbase = ic->stringToProxy(
					servername.connect("Repositories"));
	_repositories = snowstar::RepositoriesPrx::checkedCast(rbase);
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
	ui->evaluationBox->setCurrentIndex(3);
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
	ui->solverBox->setCurrentIndex(2);
	ui->solverBox->blockSignals(false);

	// get the currently active repository 
	_repository = _focusing->getRepositoryName();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repository: %s", _repository.c_str());

	// get the repository names
	ui->repositoryBox->blockSignals(true);
	ui->repositoryBox->addItem(QString());
	snowstar::reponamelist	reponames = _repositories->list();
	int	repoindex = 0;
	std::for_each(reponames.begin(), reponames.end(),
		[&](const std::string& name) {
			ui->repositoryBox->addItem(QString(name.c_str()));
			if (name == _repository) {
				repoindex = ui->repositoryBox->count();
			}
		}
	);
	ui->repositoryBox->setCurrentIndex(repoindex);
	ui->repositoryBox->blockSignals(false);

	// get the current focuser
	ui->centerSpinBox->setValue(_focuser->current());

	// emit the current state of the focusing process
	emit stateReceived(_focusing->status());

	// focuser
	ui->centerSpinBox->setMinimum(_focuser->min());
	ui->centerSpinBox->setMaximum(_focuser->max());

	// retrieve the focusing history
	if (_focusing->status() == snowstar::FocusFOCUSED) {
		snowstar::FocusHistory	history = _focusing->history();
		std::for_each(history.begin(), history.end(),
			[&](const snowstar::FocusPoint& p) {
				emit receivePoint(p);
			}
		);
	}

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
	if (_repository.size() > 0) {
		_focusing->setRepositoryName(_repository);
	}
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

/**
 * \brief Slot called when the stepsize is changed
 */
void	focusingcontrollerwidget::stepsizeChanged(int s) {
	_stepsize = s;
}

/**
 * \brief Slot called when the number of steps is changed
 */
void	focusingcontrollerwidget::stepsChanged(int s) {
	_steps = s;
}

/**
 * \brief Slot called when the center position is changed
 */
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

void    focusingcontrollerwidget::receivePoint(snowstar::FocusPoint point) {
	emit pointReceived(point);
}

void    focusingcontrollerwidget::receiveState(snowstar::FocusState state) {
	emit stateReceived(state);
}

void    focusingcontrollerwidget::receiveFocusElement(snowstar::FocusElement element) {
	emit focuselementReceived(element);
}

void	focusingcontrollerwidget::repositoryChanged(const QString& text) {
	_repository = std::string(text.toLatin1().data());
}

} // namespace snowgui
