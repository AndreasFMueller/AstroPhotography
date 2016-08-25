/*
 * serverselectiondialog.cpp -- implementation of the server selection
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "serverselectiondialog.h"
#include "ui_serverselectiondialog.h"
#include <AstroDebug.h>
#include "mainwindow.h"

using namespace astro::discover;

namespace snowgui {

ServerSelectionDialog::ServerSelectionDialog(QWidget *parent,
	ServiceDiscoveryPtr servicediscovery)
	: QDialog(parent), _servicediscovery(servicediscovery),
	  ui(new Ui::ServerSelectionDialog) {
	ui->setupUi(this);
	// get the list of all services
	ServiceDiscovery::ServiceKeySet	keys = _servicediscovery->list();
	QListWidget	*listwidget = ui->serverListWidget;
	std::for_each(keys.begin(), keys.end(),
		[listwidget](const ServiceKey& key) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %s",
				key.toString().c_str());
			listwidget->addItem(QString(key.toString().c_str()));
		}
	);
}

ServerSelectionDialog::~ServerSelectionDialog() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "desotry ServerSelectionDialog");
	delete ui;
}

void	ServerSelectionDialog::accept() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select activated");

	// selected value
	QString	qs = ui->serverListWidget->currentItem()->text();
	std::string	keystring(qs.toLatin1().data());

	// get the service key from the list
	ServiceDiscovery::ServiceKeySet	keys = _servicediscovery->list();
	ServiceDiscovery::ServiceKeySet::const_iterator	i;
	for (i = keys.begin(); i != keys.end(); i++) {
		if (keystring == i->toString()) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s",
				keystring.c_str());
			ServiceObject	so = _servicediscovery->find(*i);
			// create a main window
			MainWindow	*m = new MainWindow(NULL, so);
			m->show();
		}
	}
		
	// close the window
	close();
}

} // namespace snowgui
