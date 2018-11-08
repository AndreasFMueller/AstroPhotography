/*
 * configurationdialog.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "configurationdialog.h"
#include "ui_configurationdialog.h"
#include <CommunicatorSingleton.h>
#include <sstream>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <QMessageBox>

namespace snowgui {

/**
 * \brief construct a configuration dialog
 */
configurationdialog::configurationdialog(QWidget *parent,
	astro::discover::ServiceObject serviceobject)
	: QDialog(parent), ui(new Ui::configurationdialog),
	  _serviceobject(serviceobject) {
	ui->setupUi(this);

	_serviceobject = serviceobject;
	_servicechangewarning = false;
	_mounting = true;

	// connect to the server
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx  base = ic->stringToProxy(
				_serviceobject.connect("Configuration"));
	snowstar::ConfigurationPrx	configuration = snowstar::ConfigurationPrx::checkedCast(base);
	if (!base) {
		throw std::runtime_error("cannot create configuration app");
	}
	setConfiguration(configuration);

	// daemon connection
	base = ic->stringToProxy(_serviceobject.connect("Daemon"));
	snowstar::DaemonPrx	daemon = snowstar::DaemonPrx::checkedCast(base);
	if (!base) {
		throw std::runtime_error("cannot create daemon app");
	}
	setDaemon(daemon);

	// modules connection
	base = ic->stringToProxy(_serviceobject.connect("Modules"));
	snowstar::ModulesPrx	modules = snowstar::ModulesPrx::checkedCast(base);
	if (!base) {
		throw std::runtime_error("cannot create modules app");
	}
	setModules(modules);

	// find out whether the remote supports 
	try {
		Ice::ObjectPrx	base = ic->stringToProxy(
				_serviceobject.connect("Repositories"));
		snowstar::RepositoriesPrx	repositories
			= snowstar::RepositoriesPrx::checkedCast(base);
		ui->repositoryconfiguration->setRepositories(repositories);
	} catch (...) {
	}

	// connect buttons
	connect(ui->devicesCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(devicesToggled(bool)));
	connect(ui->imagesCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(imagesToggled(bool)));
	connect(ui->instrumentsCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(instrumentsToggled(bool)));
	connect(ui->guidingCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(guidingToggled(bool)));
	connect(ui->focusingCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(focusingToggled(bool)));
	connect(ui->repositoriesCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(repositoriesToggled(bool)));
	connect(ui->tasksCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(tasksToggled(bool)));
	connect(ui->gatewayCheckBox, SIGNAL(toggled(bool)),
		this, SLOT(gatewayToggled(bool)));

	connect(ui->restartButton, SIGNAL(clicked()),
		this, SLOT(restartClicked()));

	connect(ui->repodbField, SIGNAL(textChanged(QString)),
		this, SLOT(repodbChanged(QString)));
	connect(ui->repodbButton, SIGNAL(clicked()),
		this, SLOT(repodbClicked()));

	connect(ui->deviceField, SIGNAL(textChanged(QString)),
		this, SLOT(deviceChanged(QString)));
	connect(ui->mountpointField, SIGNAL(textChanged(QString)),
		this, SLOT(mountpointChanged(QString)));
	connect(ui->mountButton, SIGNAL(clicked()),
		this, SLOT(mountClicked()));

	connect(ui->syncButton, SIGNAL(clicked()),
		this, SLOT(syncClicked()));
	connect(ui->setButton, SIGNAL(clicked()),
		this, SLOT(setfromsourceClicked()));
	connect(ui->sourceBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(timesourceSelected(int)));

	// set up the timer
	connect(&_statusTimer, SIGNAL(timeout()), this, SLOT(timeUpdate()));
	_statusTimer.setInterval(1000);

	// connect the shutdown buttons
	connect(ui->serverButton, SIGNAL(clicked()),
		this, SLOT(shutdownClicked()));
	connect(ui->systemButton, SIGNAL(clicked()),
		this, SLOT(systemClicked()));

	// title
	setWindowTitle(QString("Configuration"));
	std::string	title = astro::stringprintf("Remote configuration on %s", _serviceobject.toString().c_str());
	ui->remoteconfigurationLabel->setText(QString(title.c_str()));
}

/**
 * \brief destroy the configuration dialog
 */
configurationdialog::~configurationdialog() {
	delete ui;
}

/**
 * \brief 
 */
bool	configurationdialog::getService(const std::string& name) {
	std::string	value("no");
	if ((name == "devices") || (name == "images")) {
		value = std::string("yes");
	}
	snowstar::ConfigurationKey	key;
	key.domain = "snowstar";
	key.section = "service";
	key.name = name;
	if (_configuration->has(key)) {
		snowstar::ConfigurationItem	item
			= _configuration->get(key);
		value = item.value;
	}
	return (value == "yes");
}

template<class T>
class Blocker {
	T	*_blocked;
public:
	Blocker(T *blocked) : _blocked(blocked) {
		_blocked->blockSignals(true);
	}
	~Blocker() { 
		_blocked->blockSignals(false);
	}
	T	*operator->() { return _blocked; }
};

template<class T> inline Blocker<T>	whileBlocking(T *blocked) {
	return Blocker<T>(blocked);
}

/**
 * \brief set a new configuration
 */
void	configurationdialog::setConfiguration(snowstar::ConfigurationPrx configuration) {
	if (!configuration) {
		return;
	}
	_configuration = configuration;

	// read the configuration information and update the widgets
	whileBlocking(ui->devicesCheckBox)->setChecked(getService("devices"));
	whileBlocking(ui->imagesCheckBox)->setChecked(getService("images"));
	whileBlocking(ui->instrumentsCheckBox)->setChecked(getService("instruments"));
	whileBlocking(ui->guidingCheckBox)->setChecked(getService("guiding"));
	whileBlocking(ui->focusingCheckBox)->setChecked(getService("focusing"));
	whileBlocking(ui->repositoriesCheckBox)->setChecked(getService("repository"));
	whileBlocking(ui->tasksCheckBox)->setChecked(getService("tasks"));
	whileBlocking(ui->gatewayCheckBox)->setChecked(getService("gateway"));

	// read the directory path
	snowstar::ConfigurationKey	key;
	key.domain = "snowstar";
	key.section = "repositories";
	key.name = "directory";
	try {
		if (_configuration->has(key)) {
			snowstar::ConfigurationItem	item = _configuration->get(key);
			ui->repodbField->setText(QString(item.value.c_str()));
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get config: %s", x.what());
	}
	
}

/**
 * \brief Method to set the dameon proxy
 */
void	configurationdialog::setDaemon(snowstar::DaemonPrx daemon) {
	if (!daemon) {
		return;
	}
	_daemon = daemon;
	ui->repositoryconfiguration->setDaemon(daemon);
}

/**
 * \brief Method to set the modules proxy, initializes mount list
 */
void	configurationdialog::setModules(snowstar::ModulesPrx modules) {
	if (!modules) {
		return;
	}
	_modules = modules;
	// construct a list of modules
	snowstar::ModuleNameList	names = modules->getModuleNames();
	for (auto i = names.begin(); i != names.end(); i++) {
		snowstar::DriverModulePrx	module = modules->getModule(*i);
		if (!module->hasLocator())
			continue;
		snowstar::DeviceLocatorPrx	locator
			= module->getDeviceLocator();
		snowstar::DeviceNameList	devnames
			= locator->getDevicelist(snowstar::DevMOUNT);
		for (auto j = devnames.begin(); j != devnames.end(); j++) {
			timesourceinfo	*tsi = new timesourceinfo();
			tsi->name = *j;
			tsi->locator = locator;
			timesourceinfoPtr	tsiptr(tsi);
			_timesources.push_back(tsiptr);
		}
	}

	// populate the source menu with the names
	ui->sourceBox->blockSignals(true);
	for (auto i = _timesources.begin(); i != _timesources.end(); i++) {
		ui->sourceBox->addItem(QString((*i)->name.c_str()));
	}
	if (ui->sourceBox->count() > 0) {
		ui->sourceBox->setCurrentIndex(0);
		timesourceSelected(0);
	}
	ui->sourceBox->blockSignals(false);
}

/**
 * \brief Method to change a configuration value
 */
void	configurationdialog::changevalue(const std::string& name,
		bool defaultvalue, bool newvalue) {
	std::string	targetvalue = (newvalue) ? "yes" : "no";
	snowstar::ConfigurationKey	key;
	key.domain = "snowstar";
	key.section = "service";
	key.name = name;
	snowstar::ConfigurationItem	item;
	if (_configuration->has(key)) {
		item = _configuration->get(key);
	} else {
		item.domain = "snowstar";
		item.section = "service";
		item.name = name;
		item.value = (defaultvalue) ? "yes" : "no";
	}
	if (targetvalue != item.value) {
		item.value = targetvalue;
		_configuration->set(item);
	}
	if (_servicechangewarning) {
		return;
	}
	ui->restartButton->setEnabled(true);
	QMessageBox	*message = new QMessageBox(this);
	message->setText(QString("Server restart required"));
	std::ostringstream	str;
	str << "Changing the service configuration requires a server restart. ";
	str << "Please exit all Snowstar appications and restart the Snowstar server process on '";
	str << _serviceobject.toString();
	str << "'.";
	message->setInformativeText(QString(str.str().c_str()));
	message->exec();
	delete message;
	_servicechangewarning = true;
}

void	configurationdialog::devicesToggled(bool newvalue) {
	changevalue("devices", true, newvalue);
}

void	configurationdialog::instrumentsToggled(bool newvalue) {
	changevalue("instruments", false, newvalue);
}

void	configurationdialog::imagesToggled(bool newvalue) {
	changevalue("images", true, newvalue);
}

void	configurationdialog::guidingToggled(bool newvalue) {
	changevalue("guiding", false, newvalue);
}

void	configurationdialog::focusingToggled(bool newvalue) {
	changevalue("focusing", false, newvalue);
}

void	configurationdialog::repositoriesToggled(bool newvalue) {
	changevalue("repository", false, newvalue);
}

void	configurationdialog::tasksToggled(bool newvalue) {
	changevalue("tasks", false, newvalue);
}

void	configurationdialog::gatewayToggled(bool newvalue) {
	changevalue("gateway", false, newvalue);
}

void	configurationdialog::restartClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "restart initiated");
	_daemon->restartServer(1.0f);
}

void	configurationdialog::repodbChanged(QString s) {
	std::string	filename = std::string(s.toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repo db changed to %s",
		filename.c_str());

	// enable the button if the filename string points to a writable file
	try {
		snowstar::FileInfo	fileinfo = _daemon->statFile(filename);
		if (fileinfo.writeable) {
			ui->repodbButton->setText("Open");
			ui->repodbButton->setEnabled(true);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found a writable file");
			return;
		}
	} catch (const std::exception& x) {
		// this is not a file
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not a file",
			filename.c_str());
	}

	// check whether the parent directory is actually a writable directory
	try {
		snowstar::DirectoryInfo	dirinfo
			= _daemon->statDirectory(filename);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found a directory");
		ui->repodbButton->setEnabled(false);
		return;
	} catch (const std::exception& x) {
		// this is not a directory
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s is not a directory",
			filename.c_str());
	}

	// strip the last component from the name
	size_t	l = filename.rfind('/');
	if (std::string::npos != l) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "l = %d", l);
		std::string	dirname = filename.substr(0, l);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dirname = '%s'",
			dirname.c_str());
		std::string	fname = filename.substr(l + 1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filename = '%s'",
			fname.c_str());
		if (fname.size() == 0) {
			ui->repodbButton->setEnabled(false);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "just a directory name");
			return;
		}
		filename = dirname;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "check directory '%s'",
		filename.c_str());

	// check whether the directory is writable
	try {
		snowstar::DirectoryInfo dirinfo
			= _daemon->statDirectory(filename);
		if (dirinfo.writeable) {
			ui->repodbButton->setText("Create");
			ui->repodbButton->setEnabled(true);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"found a creatable file %s", filename.c_str());
			return;
		}
	} catch (const std::exception& x) {
		// this file is not writable
	}

	// make sure 
	ui->repodbButton->setEnabled(false);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "nothing found");
}

void	configurationdialog::repodbClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "repodb button clicked");
	try {
		snowstar::ConfigurationItem	item;
		item.domain = "snowstar";
		item.section = "repositories";
		item.name = "directory";
		item.value = std::string(ui->repodbField->text().toLatin1().data());
		_configuration->set(item);
		_daemon->reloadRepositories();
		ui->repositoryconfiguration->readRepositories();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set directory");
	}
}

void	configurationdialog::deviceChanged(QString device) {
	std::string	devicename(device.toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deviceChanged(%s)", devicename.c_str());
	try {
		_daemon->statDevice(devicename);
		std::string	mountpoint(
				ui->mountpointField->text().toLatin1().data());
		_daemon->statDirectory(mountpoint);
		ui->mountButton->setEnabled(true);
	} catch (const std::exception& x) {
		ui->mountButton->setEnabled(false);
	}
}

void	configurationdialog::mountpointChanged(QString m) {
	std::string	mountpoint(m.toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mountpointChanged(%s)",
		mountpoint.c_str());
	try {
		_daemon->statDirectory(mountpoint);
		std::string	devicename(
				ui->deviceField->text().toLatin1().data());
		_daemon->statDevice(devicename);
		ui->mountButton->setEnabled(true);
	} catch (const std::exception& x) {
		ui->mountButton->setEnabled(false);
	}
}

void	configurationdialog::operationFailed(const std::string& s) {
        QMessageBox     message;
        message.setText(QString("Operation failed"));
        std::ostringstream      out;
        out << "The requested operation failed: ";
	out << s;
        message.setInformativeText(QString(out.str().c_str()));
        message.exec();
}

/**
 * \brief Slot called when the mount button is clicked
 */
void	configurationdialog::mountClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mountClicked()");
	std::string devicename(ui->deviceField->text().toLatin1().data());
	std::string	mountpoint(ui->mountpointField->text().toLatin1().data());
	try {
		if (_mounting) {
			_daemon->mount(devicename, mountpoint);
			ui->mountButton->setText(QString("Unmount"));
			ui->mountpointField->setEnabled(false);
			ui->deviceField->setEnabled(false);
			_mounting = false;
		} else {
			_daemon->unmount(mountpoint);
			ui->mountButton->setText(QString("Mount"));
			ui->mountpointField->setEnabled(true);
			ui->deviceField->setEnabled(true);
			_mounting = true;
		}
	} catch (const std::exception& x) {
		operationFailed(x.what());
	}
}

/**
 * \brief Slot called when "Set from source" button is clicked
 *
 * This slot sets the system time of the daemon to the currently
 * selected mount time
 */
void	configurationdialog::setfromsourceClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set from source clicked");
	if (!_mount) {
		return;
	}
	try {
		_daemon->setSystemTime(_mount->getTime());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "time set from source time");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set the time: %s",
			x.what());
	}
}

/**
 * \brief Slot called when the sync button is clicked
 */
void	configurationdialog::syncClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sync clicked");
	if (!_daemon) {
		return;
	}
	time_t	now;
	time(&now);
	try {
		_daemon->setSystemTime(now);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "time set from local time");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot sync: %s", x.what());
	}
}

static std::string	timeformat(time_t when) {
	struct tm	*tmp = localtime(&when);
	char	buffer[20];
	strftime(buffer, sizeof(buffer), "%H:%M:%S", tmp);
	return std::string(buffer);
}

/**
 * \brief Slot called by the timer to update the time information
 */
void	configurationdialog::timeUpdate() {
	// get local time and write to the localTimeField
	time_t	now;
	time(&now);
	ui->localTimeField->setText(QString(timeformat(now).c_str()));

	// get time from the remote system and write to the systemTimeField
	try {
		ui->systemTimeField->setText(QString(timeformat(
					_daemon->getSystemTime()).c_str()));
		ui->systemTimeField->setEnabled(true);
		ui->syncButton->setEnabled(true);
	} catch (const std::exception& x) {
		ui->systemTimeField->setEnabled(false);
		ui->systemTimeField->setText(QString(x.what()));
		ui->syncButton->setEnabled(false);
	}

	// get time from the selected mount and write to the sourceTimeField
	try {
		ui->sourceTimeField->setText(QString(timeformat(
			_mount->getTime()).c_str()));
		ui->sourceTimeField->setEnabled(true);
		ui->setButton->setEnabled(true);
	} catch (const std::exception& x) {
		ui->sourceTimeField->setEnabled(false);
		ui->sourceTimeField->setText(QString(x.what()));
		ui->setButton->setEnabled(false);
	}
}

/**
 * \brief Slot called when a different time source is selected
 *
 * \param index		index of the time source to consider
 */
void	configurationdialog::timesourceSelected(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "time source changed to %d", index);
	_statusTimer.stop();
	timesourceinfoPtr	tsiptr = _timesources[index];
	if (!tsiptr->mount) {
		try {
			tsiptr->mount = tsiptr->locator->getMount(tsiptr->name);
		} catch (std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot get mount: %s",
				x.what());
		}
	}
	_mount = tsiptr->mount;
	ui->setButton->setEnabled(true);
	ui->sourceTimeField->setEnabled(true);
	ui->sourceTimeLabel->setEnabled(true);
	_statusTimer.start();
}

void	configurationdialog::shutdownClicked() {
	if (!_daemon) { return; }
	QMessageBox	message;
	message.setText(QString("Server process shutdown"));
	message.setInformativeText(QString("Do you really want to shut down the server process?"));
	message.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
message.setDefaultButton(QMessageBox::Cancel);
	int     rc = message.exec();
	switch (rc) {
	case QMessageBox::Cancel:
		break;
	case QMessageBox::Ok:
		_daemon->shutdownServer(0);
		break;
	}
}

void	configurationdialog::systemClicked() {
	if (!_daemon) { return; }
	QMessageBox	message;
	message.setText(QString("Server OS shutdown"));
	message.setInformativeText(QString("Do you really want to shut down the server operating system?"));
	message.setStandardButtons(QMessageBox::Cancel | QMessageBox::Ok);
message.setDefaultButton(QMessageBox::Cancel);
	int     rc = message.exec();
	switch (rc) {
	case QMessageBox::Cancel:
		break;
	case QMessageBox::Ok:
		_daemon->shutdownSystem(0);
		break;
	}
}

} // namespace snowgui
