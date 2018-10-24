/*
 * DeviceAction.cpp -- Implementation of menu actions that open devices
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "DeviceAction.h"

namespace snowgui {

/**
 * \brief Construct a DeviceAction object
 *
 * \param devicename	name of the device
 * \param text		menu item text to display
 * \param parent	parent object
 */
DeviceAction::DeviceAction(const std::string& devicename, QString text,
	QObject *parent)
		: QAction(text, parent), _devicename(devicename) {
	connect(this, SIGNAL(triggered()), this, SLOT(doopen()));
}

/**
 * \brief Destroy the DeviceAction object
 */
DeviceAction::~DeviceAction() {
}

/**
 * \brief Slot called when the action is triggered
 *
 * This emmits the openDevice signal with the remembered devicename as
 * the parameter.
 */
void	DeviceAction::doopen() {
	emit openDevice(_devicename);
}

} // namespace snowgui
