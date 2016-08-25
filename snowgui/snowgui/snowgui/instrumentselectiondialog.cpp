/*
 * instrumentselectiondialgo.cpp -- instrument selection implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "instrumentselectiondialog.h"
#include "ui_instrumentselectiondialog.h"
#include <CommunicatorSingleton.h>

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
	close();
}

/**
 * \brief Base class method to launch the subapplication
 */
void	InstrumentSelectionDialog::launch(const std::string& instrumentname) {
	debug(LOG_ERR, DEBUG_LOG, 0, "%s: can only launch from derived class",
		instrumentname.c_str());
}

} // namespace snowgui
