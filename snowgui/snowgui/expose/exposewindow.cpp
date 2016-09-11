#include "exposewindow.h"
#include "ui_exposewindow.h"

namespace snowgui {

/**
 * \brief Construct a new Expose window
 */
exposewindow::exposewindow(QWidget *parent) : InstrumentWidget(parent),
	ui(new Ui::exposewindow) {
	ui->setupUi(this);
	ui->ccdcontrollerWidget->hideButtons(true);
	ui->ccdcontrollerWidget->imageproxyonly(true);

	// connections
	connect(ui->filterwheelcontrollerWidget,
		SIGNAL(filterwheelSelected(snowstar::FilterWheelPrx)),
		ui->exposeWidget,
		SLOT(filterwheelSelected(snowstar::FilterWheelPrx)));
	connect(ui->ccdcontrollerWidget, SIGNAL(imageproxyReceived(snowstar::ImagePrx)),
		ui->exposeWidget,
		SLOT(imageproxyReceived(snowstar::ImagePrx)));
	connect(ui->exposeWidget, SIGNAL(startExposure()),
		ui->ccdcontrollerWidget, SLOT(captureClicked()));
}

/**
 * \brief Destroy the expose window
 */
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

	// perform instrument setup in all child InstrumentWidgets
	ui->ccdcontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->focusercontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->coolercontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->filterwheelcontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->guideportcontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->adaptiveopticscontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->mountcontrollerWidget->instrumentSetup(serviceobject,
		instrument);
	ui->exposeWidget->instrumentSetup(serviceobject,
		instrument);

	// give this application a name
	setAppname("Expose");
}

/**
 * \brief Handle the close event
 */
void	exposewindow::closeEvent(QCloseEvent *) {
	deleteLater();
}

} // namespace snowgui
