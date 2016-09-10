#include "exposewindow.h"
#include "ui_exposewindow.h"

namespace snowgui {

exposewindow::exposewindow(QWidget *parent) : InstrumentWidget(parent),
	ui(new Ui::exposewindow) {
	ui->setupUi(this);
}

exposewindow::~exposewindow() {
    delete ui;
}

/**
 * \brief Instrument setup
 *
 * Propagate instrument information to all the components that need it
 */
void	exposewindow::instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument) {
	InstrumentWidget::instrumentSetup(serviceobject, instrument);

	setAppname("Expose");
}

void	exposewindow::closeEvent(QCloseEvent *) {
	deleteLater();
}

} // namespace snowgui
