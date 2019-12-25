/*
 * tasksubmissionwidget.cpp -- submit task to the task queue
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "tasksubmissionwidget.h"
#include "ui_tasksubmissionwidget.h"
#include <CommunicatorSingleton.h>
#include <IceConversions.h>
#include <tasks.h>
#include <AstroUtils.h>
#include <QMessageBox>

namespace snowgui {

/**
 * \brief create a task submission widget
 */
tasksubmissionwidget::tasksubmissionwidget(QWidget *parent)
	: InstrumentWidget(parent), ui(new Ui::tasksubmissionwidget) {
	ui->setupUi(this);

	// initialize index variables
	_ccdindex = -1;
	_coolerindex = -1;
	_filterwheelindex = -1;
	_mountindex = -1;
	_focuserindex = -1;
	_guiderccdindex = -1;
	_guideportindex = -1;
	_adaptiveopticsindex = -1;

	ui->submitButton->setEnabled(false);

	// add task type entries
	ui->tasktypeBox->addItem(QString("exposure"));
	ui->tasktypeBox->addItem(QString("dither"));
	ui->tasktypeBox->addItem(QString("focus"));
	ui->tasktypeBox->addItem(QString("sleep"));
	tasktypeChanged(0);

	// connect the tasktype button
	connect(ui->tasktypeBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(tasktypeChanged(int)));

	// connect submit button
	connect(ui->submitButton, SIGNAL(clicked()),
		this, SLOT(submitClicked()));
	connect(ui->projectField, SIGNAL(textChanged(const QString&)),
		this, SLOT(projectChanged(const QString&)));

}

/**
 * \brief destroy the task submission widget
 */
tasksubmissionwidget::~tasksubmissionwidget() {
	delete ui;
}

/**
 * \brief setup the instrument for the task submission widget
 */
void	tasksubmissionwidget::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"setting up instrument for task submission");

	// parent setup
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	// remember instrument name
	_instrumentname = instrument.name();

	// create proxy for repositories
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx  base = ic->stringToProxy(
			serviceobject.connect("Repositories"));
	snowstar::RepositoriesPrx      repositories
		= snowstar::RepositoriesPrx::checkedCast(base);
	if (!base) {
		throw std::runtime_error("cannot create repository proxy");
	}
	setRepositories(repositories);

	// create proxy for tasks
	base = ic->stringToProxy(serviceobject.connect("Tasks"));
	_tasks = snowstar::TaskQueuePrx::checkedCast(base);
	if (!_tasks) {
		throw std::runtime_error("cannot create tasks proxy");
	}

	// get all the cameras
	int	index = 0;
	while (_instrument.has(snowstar::InstrumentCamera, index)) {
		try {
			snowstar::CameraPrx camera = _instrument.camera(index);
			if (!_camera) {
				_camera = camera;
			}
			_camera_names.push_back(camera->getName());
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring camera %d: %s",
				index, x.what());
		}
		index++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument setup complete");
}

/**
 * \brief Main thread initializations
 *
 * this method fills the menu with the camera names
 */
void	tasksubmissionwidget::setupComplete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "main thread initializations");
	std::vector<std::string>::const_iterator	i;
	for (i = _camera_names.begin(); i != _camera_names.end(); i++) {
		QString	cameraname(i->c_str());
		ui->cameraBox->addItem(cameraname);
	}
}

/**
 * \brief Remember the repositories proxy and build list of repo names
 */
void	tasksubmissionwidget::setRepositories(
		snowstar::RepositoriesPrx repositories) {
	// remove all items from the repository list
	ui->repositoryBox->blockSignals(true);
	while (ui->repositoryBox->count() > 0) {
		ui->repositoryBox->removeItem(0);
	}

	// add the default entry (no repository) to the list
	ui->repositoryBox->addItem(QString("(none)"));
	ui->repositoryBox->blockSignals(false);

	// remember the repository proxy
	_repositories = repositories;
	if (!_repositories) {
		return;
	}

	// add all the repository names found in the list
	ui->repositoryBox->blockSignals(true);
	snowstar::reponamelist	repos = _repositories->list();
	snowstar::reponamelist::const_iterator	i;
	for (i = repos.begin(); i != repos.end(); i++) {
		std::string	reponame = *i;
		ui->repositoryBox->addItem(QString(reponame.c_str()));
	}
	ui->repositoryBox->blockSignals(false);
}

/**
 * \brief remember changed exposure
 */
void	tasksubmissionwidget::exposureChanged(
		astro::camera::Exposure exposure) {
	_exposure = exposure;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got new exposure info: %s",
		_exposure.toString().c_str());
}

/**
 * \brief learn about the filter wheel and update the list of filter names
 */
void	tasksubmissionwidget::filterwheelSelected(
		snowstar::FilterWheelPrx filterwheel) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got the filter wheel");
	// return if we have not filterwheel proxy
	if (!filterwheel) {
		return;
	}

	ui->filterBox->blockSignals(true);
	// clear list of filters
	while (ui->filterBox->count() > 0) {
		ui->filterBox->removeItem(0);
	}

	// update the list of filters 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "querying current filter wheel state");
	int	nfilters = filterwheel->nFilters();
	for (int i = 0; i < nfilters; i++) {
		std::string	name = filterwheel->filterName(i);
		QString	qname(name.c_str());
		ui->filterBox->addItem(qname);
	}

	// reenable signals
	ui->filterBox->blockSignals(false);
}

int	tasksubmissionwidget::warnParameters(const std::string& m) {
	QMessageBox	message;
	message.setText(QString("Warning"));
	std::ostringstream	out;
	out << "Parameter warning: " << m;
	out << " Do you really want to submit these tasks?";
	message.setInformativeText(QString(out.str().c_str()));
	message.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
	message.setDefaultButton(QMessageBox::Cancel);
	int	rc = message.exec();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "message rc=%d", rc);
	switch (rc) {
	case QMessageBox::Cancel:
		return 1;
		break;
	case QMessageBox::Ok:
		return 0;
		break;
	}
	return 1;
}

/**
 * \brief Slot activated when the submit button is clicked
 */
void	tasksubmissionwidget::submitClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submitClicked()");
	// prepare the structure for submission to the task queue
	snowstar::TaskParameters	parameters;
	parameters.instrument = _instrumentname;

	// set all the device indices (must ask the controller widgets)
	parameters.cameraIndex = ui->cameraBox->currentIndex();
	parameters.ccdIndex = _ccdindex;
	parameters.coolerIndex = _coolerindex;
	parameters.filterwheelIndex = _filterwheelindex;
	parameters.mountIndex = _mountindex;
	parameters.focuserIndex = _focuserindex;
	parameters.guiderccdIndex = _guiderccdindex;
	parameters.guideportIndex = _guideportindex;
	parameters.adaptiveopticsIndex = _adaptiveopticsindex;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"camera: %d, ccd: %d, cooler: %d, filterwheel: %d, mount: %d, "
		"focuser: %d",
		parameters.cameraIndex, parameters.ccdIndex,
		parameters.coolerIndex, parameters.filterwheelIndex,
		parameters.mountIndex, parameters.focuserIndex);

	parameters.exp.frame.origin.x = 0;
	parameters.exp.frame.origin.y = 0;
	parameters.exp.frame.size.width = 0;
	parameters.exp.frame.size.height = 0;
	parameters.exp.exposuretime = 0;
	parameters.exp.gain = 0;
	parameters.exp.limit = 0;
	parameters.exp.shutter = snowstar::ShOPEN;
	parameters.exp.purpose = snowstar::ExTEST;
	parameters.exp.mode.x = 1;
	parameters.exp.mode.y = 1;

	switch (ui->tasktypeBox->currentIndex()) {
	case 0:
		submitExposure(parameters);
		break;
	case 1:
		submitDither(parameters);
		break;
	case 2:
		submitFocus(parameters);
		break;
	case 3:
		submitSleep(parameters);
		break;
	}
}

/**
 * \brief Common work for a task submit
 */
void	tasksubmissionwidget::submitCommon(
		snowstar::TaskParameters& parameters) {
	try {
		_tasks->submit(parameters);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Task submitted");
	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf("cannot submit task: "
			"%s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		// XXX warning message
		return;
	}
}

/**
 * \brief Submit a sleep task
 *
 * This task lets the server sleep for a few seconds
 */
void	tasksubmissionwidget::submitSleep(snowstar::TaskParameters& parameters) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit a sleep task");
	parameters.type = snowstar::TaskSLEEP;
	parameters.exp.exposuretime = ui->sleeptimeSpinBox->value();
	submitCommon(parameters);
}

/**
 * \brief Submit a dither task
 */
void	tasksubmissionwidget::submitDither(
		snowstar::TaskParameters& parameters) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit a dither task");
	parameters.type = snowstar::TaskDITHER;
	parameters.exp.exposuretime = ui->waitSpinBox->value();
	parameters.ccdtemperature = ui->ditherSpinBox->value();
	submitCommon(parameters);
}

/**
 * \brief Submit a focusing task
 *
 * This type of task is not implemented yet
 */
void	tasksubmissionwidget::submitFocus(
		snowstar::TaskParameters& parameters) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"submit a focus task -> not implemented");
	parameters.type = snowstar::TaskFOCUS;
}

/**
 * \brief Submit an exposure task
 */
void	tasksubmissionwidget::submitExposure(
		snowstar::TaskParameters& parameters) {
	parameters.type = snowstar::TaskEXPOSURE;

	// get all the information about the exposure from the
	// ccdcontrollerwidget
	parameters.exp = snowstar::convert(_exposure);
	if (_exposure.exposuretime() < 5) {
		std::string	m = astro::stringprintf("The exposure time of "
			"%.3fs you have chosen seems rather short.",
			_exposure.exposuretime());
		if (warnParameters(m)) {
			return;
		}
	}

	// set the ccd temperature
	parameters.ccdtemperature = ui->temperatureBox->value() + astro::Temperature::zero;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd temperature: %f",
		parameters.ccdtemperature);

	// set the filter name
	if (ui->filterBox->count() > 0) {
		parameters.filter = std::string(
			ui->filterBox->currentText().toLatin1().data());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter name: %s",
		parameters.filter.c_str());

	// set the project name
	parameters.project = std::string(
		ui->projectField->text().toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "project name: %s",
		parameters.project.c_str());

	// set the repository name
	std::string	reponame = std::string(
		ui->repositoryBox->currentText().toLatin1().data());
	if (reponame != "(none)") {
		parameters.repository = reponame;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repository name: %s",
		parameters.repository.c_str());

	// number of tasks to submit
	int	repeats = ui->exposuresBox->value();
	int	counter = 0;
	while (repeats--) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "submitting task %d", ++counter);
		submitCommon(parameters);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "task %d submitted", counter);
	}
}

/**
 * \brief set the new ccd index
 */
void	tasksubmissionwidget::ccdSelected(int ccdindex) {
	_ccdindex = ccdindex;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "CCD index: %d", _ccdindex);
}

void	tasksubmissionwidget::coolerSelected(int coolerindex) {
	_coolerindex = coolerindex;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Cooler index: %d", _coolerindex);
}

void	tasksubmissionwidget::filterwheelSelected(int filterwheelindex) {
	_filterwheelindex = filterwheelindex;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Filterwheel index: %d",
		_filterwheelindex);
}

void	tasksubmissionwidget::mountSelected(int mountindex) {
	_mountindex = mountindex;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Mount index: %d", _mountindex);
}

void	tasksubmissionwidget::focuserSelected(int focuserindex) {
	_focuserindex = focuserindex;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Focuser index: %d", _focuserindex);
}

void	tasksubmissionwidget::projectChanged(const QString& p) {
	_projectname = p;
	std::string	s(_projectname.toLatin1().data());
	s = astro::trim(s);
	ui->submitButton->setEnabled(s.size() > 0);
}

void	tasksubmissionwidget::tasktypeChanged(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task type changed to %d", index);
	switch (index) {
	case 0:
		ui->exposureParameterWidget->setVisible(true);
		ui->ditherParameterWidget->setVisible(false);
		ui->focusParameterWidget->setVisible(false);
		ui->sleepParameterWidget->setVisible(false);
		{
			std::string	s(ui->projectField->text().toLatin1().data());
			s = astro::trim(s);
			ui->submitButton->setEnabled(s.size() > 0);
		}
		break;
	case 1:
		ui->exposureParameterWidget->setVisible(false);
		ui->ditherParameterWidget->setVisible(true);
		ui->focusParameterWidget->setVisible(false);
		ui->sleepParameterWidget->setVisible(false);
		ui->submitButton->setEnabled(true);
		break;
	case 2:
		ui->exposureParameterWidget->setVisible(false);
		ui->ditherParameterWidget->setVisible(false);
		ui->focusParameterWidget->setVisible(true);
		ui->sleepParameterWidget->setVisible(false);
		ui->submitButton->setEnabled(false);
		break;
	case 3:
		ui->ditherParameterWidget->setVisible(false);
		ui->exposureParameterWidget->setVisible(false);
		ui->focusParameterWidget->setVisible(false);
		ui->sleepParameterWidget->setVisible(true);
		ui->submitButton->setEnabled(true);
		break;
	}
}

} // namespace snowgui
