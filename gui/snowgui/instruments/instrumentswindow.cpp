/*
 * instrumentswindow.cpp -- implementation of instruments editor window
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "instrumentswindow.h"
#include "ui_instrumentswindow.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <CommunicatorSingleton.h>
#include <QMessageBox>

namespace snowgui {

/**
 * \brief Create a new instrumentswindow
 */
instrumentswindow::instrumentswindow(QWidget *parent,
	const astro::discover::ServiceObject serviceobject)
	: QWidget(parent), ui(new Ui::instrumentswindow),
	  _serviceobject(serviceobject) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating an instrumentswindow");
	// create he userinterface
	ui->setupUi(this);

	// get service discover
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start discovery");
	_discovery = astro::discover::ServiceDiscovery::get();
	_discoveryTimer = new QTimer();
	_discoveryTimer->setInterval(1000);

	// connections
	connect(ui->instrumentselectionBox,
		SIGNAL(currentIndexChanged(QString)),
		this, SLOT(instrumentSelected(QString)));
	connect(ui->serverselectionBox,
		SIGNAL(currentIndexChanged(QString)),
		this, SLOT(serviceSelected(QString)));
	connect(ui->deleteinstrumentButton, SIGNAL(clicked()),
		this, SLOT(deleteInstrument()));

	connect(_discoveryTimer, SIGNAL(timeout()),
		this, SLOT(checkdiscovery()));

	connect(ui->deleteButton, SIGNAL(clicked()),
		this, SLOT(deleteClicked()));
	connect(ui->addButton, SIGNAL(clicked()),
		this, SLOT(addClicked()));
	connect(ui->addguiderccdButton, SIGNAL(clicked()),
		this, SLOT(addguiderccdClicked()));
	connect(ui->addfinderccdButton, SIGNAL(clicked()),
		this, SLOT(addfinderccdClicked()));

	// set the window title on
	std::string	title = astro::stringprintf("Edit instruments in %s",
		_serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	// create an interface to the instruments on that service
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
				_serviceobject.connect("Instruments"));
	_instruments = snowstar::InstrumentsPrx::checkedCast(base);
	instrumentEnabled(false);

	// read the list of instrument names from the proxy
	snowstar::InstrumentList	il = _instruments->list();
	QComboBox	*isb = ui->instrumentselectionBox;
	std::for_each(il.begin(), il.end(),
		[isb](const std::string& instrument) {
			isb->addItem(QString(instrument.c_str()));
		}
	);

	// read the service keys and add them to the the service selection
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start discover");
	checkdiscovery();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "discover complete");

	// start the timer
	_discoveryTimer->start();
}

/**
 * \brief Destroy the instrumentswindow
 */
instrumentswindow::~instrumentswindow() {
	if (_discoveryTimer) {
		_discoveryTimer->stop();
		delete _discoveryTimer;
	}
	delete ui;
}

/**
 * \brief enable/disable the buttons in the dialog
 */
void	instrumentswindow::instrumentEnabled(bool enabled) {
	ui->addButton->setEnabled(enabled);
	ui->addguiderccdButton->setEnabled(enabled);
	ui->addfinderccdButton->setEnabled(enabled);
	ui->deleteButton->setEnabled(enabled);
	ui->deleteinstrumentButton->setEnabled(enabled);
	ui->instrumentdisplayWidget->setEnabled(enabled);
}

/**
 * \brief Slot called when an instrument is selected
 *
 * This method is called then a different instrument is selected
 * from the list of instruments
 */
void	instrumentswindow::instrumentSelected(QString name) {
	_instrument = _instruments->get(std::string(name.toLatin1().data()));
	instrumentEnabled(true);

	// XXX make sure everything we know about this instrument is displayed
        ui->instrumentdisplayWidget->setInstrument(_instrument);

}

/**
 * \brief Slot called when a different service is selected
 */
void	instrumentswindow::serviceSelected(QString name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connecting to service %s",
		name.toLatin1().data());

	// turn name into a service object
	std::string	sn(name.toLatin1().data());
	_modulekey = astro::discover::ServiceKey(sn);
	astro::discover::ServiceObject	so = _discovery->find(_modulekey);

	// show modules on that server
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(so.connect("Modules"));
	_modules = snowstar::ModulesPrx::checkedCast(base);

	// tell the module display to work with this modules interface
	ui->moduledisplayWidget->setModules(_modules);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service connection successful");
}

/**
 * \brief Slot called when the timer fires
 *
 * This slot checks whether the set of services has changed and udpates
 * the combobox. It takes care not to change the currently selected
 * service.
 */
void	instrumentswindow::checkdiscovery() {
	astro::discover::ServiceDiscovery::ServiceKeySet	keys
		= _discovery->list();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rebuilding discovery list: %d items",
		keys.size());

	QComboBox	*ssb = ui->serverselectionBox;

	// if the number of entries has not changed, we stay where we are
	if (ssb->count() == keys.size()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no change in discovery");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rebuilding the menu list");

	// discovery has changed, so we have to rebuild the serverselection
	// combobox
	QString	current;
	int	previouscount = ssb->count();
	if (previouscount > 0) {
		current = ssb->currentText();
	}

	// we rebuild the list, but we have to block signals while we do
	// that, so that we don't trigger modules queries
	ssb->blockSignals(true);

	while (ssb->count()) {
		ssb->removeItem(0);
	}

	// add all the keys we have discovered to the list
	std::for_each(keys.begin(), keys.end(),
		[ssb](const astro::discover::ServiceKey& key) {
			ssb->addItem(QString(key.toString().c_str()));
		}
	);

	ssb->blockSignals(false);

	if (ssb->count() == 0) {
		if (previouscount > 0) {
			// XXX reset the modulesdisplay
		}
		return;
	}

	if (previouscount == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "selecting item 0: %s",
			ssb->itemText(0).toLatin1().data());
		if (ssb->currentIndex() != 0) {
			ssb->setCurrentIndex(0);
		} else {
			serviceSelected(ssb->currentText());
		}
		return;
	}

	for (int i = 0; i < ssb->count(); i++) {
		if (ssb->itemText(i) == current) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "selecting item %d: %s",
				i, ssb->itemText(i).toLatin1().data());
			if (i != ssb->currentIndex()) {
				ssb->setCurrentIndex(i);
			} else {
				serviceSelected(ssb->currentText());
			}
			return;
		}
	}
	ssb->setCurrentIndex(0);
	serviceSelected(ssb->currentText());
}

/**
 * \brief Slot called when the user clicks Add for a device
 */
void	instrumentswindow::addClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add clicked");
	std::string	devicename = ui->moduledisplayWidget->selectedDevicename();
	std::string	servicename = _modulekey.name();
	ui->instrumentdisplayWidget->add(devicename, servicename);
}

/**
 * \brief Slot called when the user clicks Add GuiderCCD
 */
void	instrumentswindow::addguiderccdClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add GuiderCDD clicked");
	std::string	devicename = ui->moduledisplayWidget->selectedDevicename();
	std::string	servicename = _serviceobject.name();
	ui->instrumentdisplayWidget->addGuiderCCD(devicename, servicename);
}

/**
 * \brief Slot called when the user clicks Add FinderCCD
 */
void	instrumentswindow::addfinderccdClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add GuiderCDD clicked");
	std::string	devicename = ui->moduledisplayWidget->selectedDevicename();
	std::string	servicename = _serviceobject.name();
	ui->instrumentdisplayWidget->addFinderCCD(devicename, servicename);
}

/**
 * \brief Slot called when the user wants to delete an item
 */
void	instrumentswindow::deleteClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete clicked");
        ui->instrumentdisplayWidget->deleteSelected();
}

/**
 * \brief Slot used to delete an instrument
 */
void	instrumentswindow::deleteInstrument() {
	if (!_instrument) {
		return;
	}
	std::string	name = _instrument->name();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete '%s' requested", name.c_str());
	QMessageBox	*messagebox = new QMessageBox(this);
        messagebox->setText(QString("Delete Instrument?"));
	std::string	msg = astro::stringprintf("Do you really want to delete the instrument named '%s'", name.c_str());
        messagebox->setInformativeText(QString(msg.c_str()));
	messagebox->addButton(QString("Cancel"), QMessageBox::RejectRole);
	messagebox->addButton(QString("Delete"), QMessageBox::AcceptRole);
        int	rc = messagebox->exec();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "button rc = %d", rc);
	if (rc == 1) {
		try {
			_instruments->remove(_instrument->name());
			int	index = ui->instrumentselectionBox->currentIndex();
			ui->instrumentselectionBox->removeItem(index);
			// make sure selection is consistent
			debug(LOG_DEBUG, DEBUG_LOG, 0, "current index: %d",
				ui->instrumentselectionBox->currentIndex());
			if (ui->instrumentselectionBox->currentIndex() < 0) {
				instrumentEnabled(false);
			}
		} catch (const std::exception& x) {
		}
	}
	
	delete messagebox;
}

/**
 * \brief stop the timer when the window closes
 */
void    instrumentswindow::closeEvent(QCloseEvent * /* event */) {
	_discoveryTimer->stop();
}

} // namespace snowgui
