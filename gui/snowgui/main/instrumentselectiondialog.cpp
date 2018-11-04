/*
 * instrumentselectiondialgo.cpp -- instrument selection implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "instrumentselectiondialog.h"
#include "ui_instrumentselectiondialog.h"
#include <CommunicatorSingleton.h>
#include "mainwindow.h"
#include "../icegui/InstrumentWidget.h"
#include "WindowsMenu.h"

using namespace astro::discover;

namespace snowgui {

/**
 * \brief Constructor for the instrument selection dialog
 */
InstrumentSelectionDialog::InstrumentSelectionDialog(QWidget *parent,
	ServiceObject serviceobject)
	: QDialog(parent), _serviceobject(serviceobject),
	  ui(new Ui::InstrumentSelectionDialog) {
	ui->setupUi(this);

	// build a connection to the instruments service of the server
	// and start 
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(_serviceobject.connect(
							"Instruments"));
	instruments = snowstar::InstrumentsPrx::checkedCast(base);

	// get a list of instruments available and add the as items 
	// to the list
	snowstar::InstrumentList	list = instruments->list();
	QListWidget	*listwidget = ui->instrumentListWidget;
	std::for_each(list.begin(), list.end(),
		[listwidget](const std::string& instrumentname) {
			listwidget->addItem(QString(instrumentname.c_str()));
		}
	);

	// double click on instrument should select the instrument
	connect(ui->instrumentListWidget,
		SIGNAL(itemDoubleClicked(QListWidgetItem*)),
		this,
		SLOT(accept()));
}

/**
 * \brief Destructor for instrument selection
 */
InstrumentSelectionDialog::~InstrumentSelectionDialog() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy dialog");
	delete ui;
}

/**
 * \brief Method called when the input is accepted
 */
void	InstrumentSelectionDialog::accept() {
	QString	i = ui->instrumentListWidget->currentItem()->text();
	std::string	instrumentname(i.toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "accept instrument %s",
		instrumentname.c_str());
	this->launch(instrumentname);
}

/**
 * \brief Base class method to launch the subapplication
 */
void	InstrumentSelectionDialog::launch(const std::string& instrumentname) {
	debug(LOG_ERR, DEBUG_LOG, 0, "%s: can only launch from derived class",
		instrumentname.c_str());
}

void	InstrumentSelectionDialog::launch(const std::string& instrumentname,
		InstrumentWidget *a) {
	snowstar::RemoteInstrument      ri(instruments, instrumentname);
	// get the main window and connect the offerImage signal
	// to the imageForSaving option
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connect offerImage()");

	// add the application to the menu
	//WindowsMenu::get()->add(a, "new Application");

	// start the instrument setup thread
	a->launchInstrumentSetup(_serviceobject, ri);

	// make the application visible
	a->show();
	QApplication::setActiveWindow(a);
	a->raise();

	// add the application to the menu
	try {
		WindowsMenu::get()->add(a, QString(instrumentname.c_str()));
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot add menu: %s", x.what());
	}

	// now close the selection dialog
	debug(LOG_DEBUG, DEBUG_LOG, 0, "close the selection dialog");
	close();
}

} // namespace snowgui
