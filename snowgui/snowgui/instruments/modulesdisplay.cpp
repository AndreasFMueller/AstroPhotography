/*
 * modulesdisplay.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "modulesdisplay.h"
#include "ui_modulesdisplay.h"
#include <QListWidgetItem>
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Create a new modulesdisplay
 */
modulesdisplay::modulesdisplay(QWidget *parent)
	: QWidget(parent), ui(new Ui::modulesdisplay) {
	ui->setupUi(this);

	// connections
	connect(ui->moduleselectionBox, SIGNAL(currentIndexChanged(QString)),
		this, SLOT(moduleChanged(QString)));
}

/**
 * \brief Destroy the modulesdisplay
 */
modulesdisplay::~modulesdisplay() {
	delete ui;
}

/**
 * \brief Rebuild the modules menu from a new proxy
 */
void	modulesdisplay::setModules(snowstar::ModulesPrx modules) {
	_modules = modules;
	snowstar::ModuleNameList	names = _modules->getModuleNames();
	QComboBox	*msb = ui->moduleselectionBox;

	msb->blockSignals(true);

	while (msb->count() > 0) {
		msb->removeItem(0);
	}

	std::for_each(names.begin(), names.end(),
		[msb](const std::string& modulename) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found module %s", 
				modulename.c_str());
			msb->addItem(QString(modulename.c_str()));
		}
	);

	msb->blockSignals(false);
	msb->setCurrentIndex(0);
}

/**
 * \brief add devices of a given type
 */
void	modulesdisplay::add(snowstar::DeviceLocatorPrx locator,
			enum snowstar::devicetype type) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding type %d devices", type);
	QListWidget	*lw = ui->componentList;
	snowstar::DeviceNameList	list;
	list = locator->getDevicelist(type);
	std::for_each(list.begin(), list.end(),
		[lw](const std::string& devicename) {
			QString	dl(devicename.c_str());
			QListWidgetItem	*item = new QListWidgetItem(dl);
			item->setFont(QFont("Fixed"));
			lw->addItem(item);
		}
	);
}

/**
 * \brief Switch to a different module
 */
void	modulesdisplay::moduleChanged(QString modulename) {
	std::string	_modulename(modulename.toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "switch to module %s",
		_modulename.c_str());

	QListWidget	*lw = ui->componentList;
	lw->blockSignals(true);

	// remove all previous items from the list
	while (lw->count() > 0) {
		QListWidgetItem	*w = lw->takeItem(0);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "removing item %s",
			w->text().toLatin1().data());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "list now empty: %d",
		lw->count());

	// get driver module and locator for this module
	snowstar::DriverModulePrx	drivermodule
		= _modules->getModule(_modulename);
	if (!drivermodule->hasLocator()) {
		ui->componentList->blockSignals(false);
		return;
	}
	snowstar::DeviceLocatorPrx	locator
		= drivermodule->getDeviceLocator();

	add(locator, snowstar::DevAO);
	add(locator, snowstar::DevCAMERA);
	add(locator, snowstar::DevCCD);
	add(locator, snowstar::DevCOOLER);
	add(locator, snowstar::DevFILTERWHEEL);
	add(locator, snowstar::DevFOCUSER);
	add(locator, snowstar::DevGUIDEPORT);
	add(locator, snowstar::DevMOUNT);

	ui->componentList->blockSignals(false);
}

QListWidgetItem	*modulesdisplay::selectedItem() {
	QList<QListWidgetItem *>	list
		= ui->componentList->selectedItems();
	if (list.count() == 0) {
		return NULL;
	}
	return list.first();
}


bool	modulesdisplay::deviceSelected() {
	return (NULL != selectedItem());
}

std::string	modulesdisplay::selectedDevicename() {
	QListWidgetItem	*lwi = selectedItem();
	if (NULL == lwi) {
		return std::string();
	}
	std::string	result(lwi->text().toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "selected device: %s",
		result.c_str());
	return result;
}

} // namespace snowgui
