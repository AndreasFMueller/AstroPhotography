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

	// connections
	connect(ui->instrumentselectionBox,
		SIGNAL(currentIndexChanged(QString)),
		this, SLOT(instrumentSelected(QString)));

	// set the window title on
	std::string	title = astro::stringprintf("Edit instruments in %s",
		_serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	// create an interface to the instruments on that service
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
				_serviceobject.connect("Instruments"));
	_instruments = snowstar::InstrumentsPrx::checkedCast(base);

	// read the list of instrument names from the proxy
	snowstar::InstrumentList	il = _instruments->list();
	QComboBox	*isb = ui->instrumentselectionBox;
	std::for_each(il.begin(), il.end(),
		[isb](const std::string& instrument) {
			isb->addItem(QString(instrument.c_str()));
		}
	);

}

instrumentswindow::~instrumentswindow() {
	delete ui;
}

void	instrumentswindow::instrumentSelected(QString name) {
	_instrument = _instruments->get(std::string(name.toLatin1().data()));

	// XXX make sure everything we know about this instrument is displayed
        ui->instrumentdisplayWidget->setInstrument(_instrument);
}

} // namespace snowgui
