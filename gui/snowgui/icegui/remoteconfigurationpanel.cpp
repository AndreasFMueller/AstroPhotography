/*
 * remoteconfigurationpanel.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "remoteconfigurationpanel.h"
#include "ui_remoteconfigurationpanel.h"
#include <CommunicatorSingleton.h>

namespace snowgui {

remoteconfigurationpanel::remoteconfigurationpanel(QWidget *parent)
	: QWidget(parent), ui(new Ui::remoteconfigurationpanel) {
	ui->setupUi(this);
}

remoteconfigurationpanel::~remoteconfigurationpanel() {
	delete ui;
}

void	remoteconfigurationpanel::setServiceObject(
		astro::discover::ServiceObjectPtr serviceobject) {
	if (!serviceobject) {
		return;
	}

	// get a connection to the daemon
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
		serviceobject->connect("Daemon"));
	if (!base) {
		throw std::runtime_error("cannot create daemon app");
	}
	snowstar::DaemonPrx	daemon = snowstar::DaemonPrx::checkedCast(base);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found remote daemon");

	// get the remote version information
	ui->remoteosField->setText(QString(daemon->osVersion().c_str()));
	ui->astroversionField->setText(QString(daemon->astroVersion().c_str()));
	ui->snowstarversionField->setText(QString(daemon->snowstarVersion().c_str()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "daemon information set");

	// create
	base = ic->stringToProxy(serviceobject->connect("Configuration"));
	if (!base) {
		throw std::runtime_error("cannot create daemon app");
	}
	snowstar::ConfigurationPrx      configuration
		= snowstar::ConfigurationPrx::checkedCast(base);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting configuration in remote");
	ui->remoteConfiguration->setConfiguration(configuration);
}

} // namespace snowgui
