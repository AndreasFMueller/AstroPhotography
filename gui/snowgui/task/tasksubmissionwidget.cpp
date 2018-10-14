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

	// connect submit button
	connect(ui->submitButton, SIGNAL(clicked()),
		this, SLOT(submitClicked()));
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up instrument for task sub");

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
	int	nfilters = filterwheel->nFilters();
	for (int i = 0; i < nfilters; i++) {
		std::string	name = filterwheel->filterName(i);
		QString	qname(name.c_str());
		ui->filterBox->addItem(qname);
	}

	// reenable signals
	ui->filterBox->blockSignals(false);
}

/**
 * \brief Slot activated when the submit button is clicked
 */
void	tasksubmissionwidget::submitClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submitClicked()");
	// prepare the structure for submission to the task queue
	snowstar::TaskParameters	parameters;
	parameters.instrument = _instrumentname;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "instrument: %s",
		parameters.instrument.c_str());

	// set all the device indices (must ask the controller widgets)
	parameters.cameraIndex = ui->cameraBox->currentIndex();
	parameters.ccdIndex = _ccdindex;
	parameters.coolerIndex = _coolerindex;
	parameters.filterwheelIndex = _filterwheelindex;
	parameters.mountIndex = _mountindex;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"camera: %d, ccd: %d, cooler: %d, filterwheel: %d, mount: %d",
		parameters.cameraIndex, parameters.ccdIndex,
		parameters.coolerIndex, parameters.filterwheelIndex,
		parameters.mountIndex);

	// get all the information about the exposure from the
	// ccdcontrollerwidget
	parameters.exp = snowstar::convert(_exposure);

	// set the ccd temperature
	parameters.ccdtemperature = ui->temperatureBox->value() + 273.15;
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
		try {
			_tasks->submit(parameters);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "Task %d submitted",
				counter);
		} catch (const std::exception& x) {
			std::string	msg = astro::stringprintf("cannot submit task: %s", x.what());
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			// XXX warning message
			return;
		}
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

} // namespace snowgui
