/*
 * DeviceAction.cpp -- Implementation of menu actions that open devices
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "DeviceAction.h"

namespace snowgui {

DeviceAction::DeviceAction(const std::string& devicename, QString text,
	QObject *parent)
		: QAction(text, parent), _devicename(devicename) {
	connect(this, SIGNAL(triggered()), this, SLOT(doopen()));
}

DeviceAction::~DeviceAction() {
}

void	DeviceAction::doopen() {
	emit openDevice(_devicename);
}

} // namespace snowgui
