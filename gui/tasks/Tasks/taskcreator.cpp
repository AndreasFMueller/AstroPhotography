/*
 * taskcreator.cpp -- TaskCreator implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "taskcreator.h"
#include "ui_taskcreator.h"
#include <AstroDebug.h>
#include <stdexcept>

/**
 * \brief Create a new task creator object
 */
TaskCreator::TaskCreator(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TaskCreator)
{
    ui->setupUi(this);
	// get a Modules reference
	CosNaming::Name	name;
        name.length(2);
        name[0].id = "Astro";
        name[0].kind = "context";
        name[1].id = "Modules";
        name[1].kind = "object";

	CORBA::Object_var	obj
                = ConnectionDialog::namingcontext->resolve(name);
	modules = Astro::Modules::_narrow(obj);
        if (CORBA::is_nil(modules)) {
                throw std::runtime_error("nil object reference");
        }
        debug(LOG_DEBUG, DEBUG_LOG, 0, "got a reference to a Modules object");

	// find out what cameras there are
	ui->cameraComboBox->set(modules, Astro::DeviceLocator::DEVICE_CAMERA);
	selectCamera(0);

	// find a list of filter wheels
	ui->filterwheelComboBox->addItem(QString("none"));
	ui->filterwheelComboBox->set(modules,
		Astro::DeviceLocator::DEVICE_FILTERWHEEL);
	selectFilterwheel(0);
}

static std::string	qstring2string(QString qstring) {
	QByteArray	ba = qstring.toLocal8Bit();	
	return std::string(ba.data());
}

/**
 * \brief Get the device locator for the object name
 */
Astro::DeviceLocator_var	TaskCreator::getDeviceLocator(const std::string& name) {
	// extract the module name
	if (name.find(':') == std::string::npos) {
		return Astro::DeviceLocator_var(NULL);
	}
	std::string	devname = name.substr(name.find(':') + 1);
	if (devname.find('/') != std::string::npos) {
		devname = devname.substr(0, devname.find('/'));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "module name: %s", devname.c_str());

	// prepare a device locator
	Astro::DeviceLocator_var	devicelocator;

	// ask modules and find out whether this thing has a device locator
	CORBA::String_var	drivername = CORBA::string_dup(devname.c_str());
	Astro::DriverModule_var driver = modules->getModule(drivername);
	if (CORBA::is_nil(driver)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "no driver module for %s",
			devname.c_str());
		return devicelocator._retn();
	}

	// get the descriptor
	Astro::Descriptor_var	descriptor = driver->getDescriptor();
	if (!descriptor->hasDeviceLocator) {
		return devicelocator._retn();
	}

	// get the descriptor
	devicelocator = driver->getDeviceLocator();

	// retrieve the device locator
	return devicelocator._retn();
}

/**
 * \brief Get a Camera reference
 */
Astro::Camera_var	TaskCreator::getCamera(const std::string& cameraname) {
	Astro::DeviceLocator_var	dev = getDeviceLocator(cameraname);
	if (CORBA::is_nil(dev)) {
		return Astro::Camera_var(NULL);
	}

	// retrieve the camera from the device locator
	CORBA::String_var	s = CORBA::string_dup(cameraname.c_str());
	Astro::Camera_var	camera = dev->getCamera(s);
	return camera._retn();
}

/**
 * \brief Get a FilterWheel reference
 */
Astro::FilterWheel_var	TaskCreator::getFilterwheel(
		const std::string& filterwheelname) {
	Astro::DeviceLocator_var	dev = getDeviceLocator(filterwheelname);
	Astro::FilterWheel_var	filterwheel;
	if (CORBA::is_nil(dev)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no device locator");
		return filterwheel;
	}

	CORBA::String_var	s = CORBA::string_dup(filterwheelname.c_str());
	filterwheel = dev->getFilterWheel(s);
	return filterwheel._retn();
}

/**
 * \brief Select a CCD, updates parameters input widgets
 */
void	TaskCreator::selectCcd(int ccdid) {
	ccdinfo = camera->getCcdinfo(ccdid);

	// set binning modes
	ui->binningComboBox->clear();
	for (int i = 0; i < ccdinfo->binningmodes.length(); i++) {
		char	buffer[64];
		snprintf(buffer, sizeof(buffer), "%dx%d",
			ccdinfo->binningmodes[i].x,
			ccdinfo->binningmodes[i].y);
		ui->binningComboBox->addItem(QString(buffer));
	}

	// find out whether the CCD has a shutter
	if (ccdinfo->shutter) {
		ui->lightRadioButton->setEnabled(true);
	} else {
		ui->lightRadioButton->setEnabled(false);
		ui->lightRadioButton->setChecked(true);
	}

	Astro::Ccd_var	ccd = camera->getCcd(0);
	ui->temperatureLabel->setEnabled(ccd->hasCooler());
	ui->temperatureSpinBox->setEnabled(ccd->hasCooler());
}

/**
 * \brief Select a camera, updates parameter input widgets
 */
void	TaskCreator::selectCamera(int cameraposition) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select camera %d", cameraposition);
	// select camera, and determine the number of CCDs it has
	std::string	cameraname = qstring2string(
		ui->cameraComboBox->itemText(cameraposition));
	camera = getCamera(cameraname);
	if (CORBA::is_nil(camera)) {
		return;
	}

	// find out how many ccds this camera has
	int	nccds = camera->nCcds();
	ui->ccdSpinBox->setMaximum(nccds);

	// get information about the CCD
	selectCcd(0);
}

/**
 * \brief Select a FilterWheel, updates parameter input widgets
 */
void	TaskCreator::selectFilterwheel(int filterwheelposition) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select filterwheel %d",
		filterwheelposition);
	if (0 == filterwheelposition) {
		ui->filterpositionComboBox->clear();
		ui->filterpositionComboBox->setEnabled(false);
		ui->positionLabel->setEnabled(false);
		return;
	}

	ui->filterpositionComboBox->setEnabled(true);
	ui->positionLabel->setEnabled(false);

	// get the filterwheel name
	std::string	filterwheelname = qstring2string(
		ui->filterwheelComboBox->itemText(filterwheelposition));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve filterwheel %s",
		filterwheelname.c_str());
	Astro::FilterWheel_var	filterwheel = getFilterwheel(filterwheelname);
	if (CORBA::is_nil(filterwheel)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no filter wheel");
		return;
	}

	// retrieve number of filter positions from the filter wheel
	ui->filterpositionComboBox->clear();
	int	nfilters = filterwheel->nFilters();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel has %d filters", nfilters);
	for (int i = 0; i < nfilters; i++) {
		char	buffer[128];
		char	*filtername = filterwheel->filterName(i);
		snprintf(buffer, sizeof(buffer), "%d: %s", i, filtername);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "position %d: %s", i, buffer);
		ui->filterpositionComboBox->addItem(QString(buffer));
	}
}

/**
 * \brief Submit a new task
 */
void	TaskCreator::submitTask(int multiplicity) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "submit new task %d times", multiplicity);

	// first construct the parameters
	Astro::TaskParameters	parameters;

	QString	cameraname = ui->cameraComboBox->currentText();
	std::string	camerastring = qstring2string(cameraname);
	parameters.camera = CORBA::string_dup(camerastring.c_str());

	parameters.ccdid = ui->ccdSpinBox->value();

	if (ui->filterwheelComboBox->currentIndex()) {
		QString	filterwheelname
			= ui->filterwheelComboBox->currentText();
		std::string	filterwheelstring
			= qstring2string(filterwheelname);
		parameters.filterwheel
			= CORBA::string_dup(filterwheelstring.c_str());
		parameters.filterposition
			= ui->filterpositionComboBox->currentIndex();
	} else {
		parameters.filterwheel = CORBA::string_dup("");
		parameters.filterposition = 0;
	}

	// ccd temperature
	parameters.ccdtemperature = ui->temperatureSpinBox->value() + 273.15;

	// exposure time
	parameters.exp.exposuretime = ui->exposureTime->value();

	// frame to expose (no user interface yet)
	parameters.exp.frame.size.width = 0;
	parameters.exp.frame.size.height = 0;
	parameters.exp.frame.origin.x = 0;
	parameters.exp.frame.origin.y = 0;

	// gain and limit (no user interface)
	parameters.exp.gain = 1;
	parameters.exp.limit = 10000000;
	parameters.exp.shutter = (ui->lightRadioButton->isChecked())
					? Astro::SHUTTER_OPEN
					: Astro::SHUTTER_CLOSED;

	// binning mode
	int	binning = ui->binningComboBox->currentIndex();
	parameters.exp.mode.x = ccdinfo->binningmodes[binning].x;
	parameters.exp.mode.y = ccdinfo->binningmodes[binning].y;

	// submit the job
	while (multiplicity--) {
		long	id = _taskqueue->submit(parameters);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new task %d submitted", id);
	}
}

/**
 * \brief Destroy the task creator
 */
TaskCreator::~TaskCreator()
{
    delete ui;
}
