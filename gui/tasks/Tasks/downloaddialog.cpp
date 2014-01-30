/**
 * downloaddialog.cpp -- Dialog to request file name generation parameters
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "downloaddialog.h"
#include "ui_downloaddialog.h"
#include <AstroDebug.h>

/**
 * \brief Construct a DownloadDialog
 *
 * \param _parameters	The parameters for file name generation
 */
DownloadDialog::DownloadDialog(DownloadParameters& _parameters,
	QWidget *parent)
	: QDialog(parent), parameters(_parameters),
	  ui(new Ui::DownloadDialog) {

	ui->setupUi(this);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "parameters received: %s",
		parameters.toString().c_str());

	// set the parameters in the UI
	ui->prefixField->setText(parameters.prefix);
	ui->exposureButton->setChecked(parameters.exposuretime);
	ui->temperatureButton->setChecked(parameters.temperature);
	ui->binningButton->setChecked(parameters.binning);
	ui->lightButton->setChecked(parameters.shutter);
	ui->filterButton->setChecked(parameters.filter);
	ui->dateButton->setChecked(parameters.date);
	
	// connect signals to slots
	connect(this, SIGNAL(accepted()),
		this, SLOT(acceptedSlot()));
	connect(this, SIGNAL(accepted()),
		parent, SLOT(downloadParametersAccepted()),
		Qt::QueuedConnection);
}

/**
 * \brief Destroy the DownloadDialog
 */
DownloadDialog::~DownloadDialog() {
	delete ui;
}

/**
 * \brief Slot called when the text changes
 */
void	DownloadDialog::textChanged(const QString& text) {
	parameters.prefix = text;
}

/**
 * \brief Slot called when the date is toggled
 */
void	DownloadDialog::dateToggled(bool checked) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "date: %s",
		(checked) ? "YES" : "NO");
	parameters.date = checked;
}

/**
 * \brief Slot called when the exposure button is toggled
 */
void	DownloadDialog::exposureToggled(bool checked) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "exposure: %s",
		(checked) ? "YES" : "NO");
	parameters.exposuretime = checked;
}

/**
 * \brief Slot called when the binning button is toggled
 */
void	DownloadDialog::binningToggled(bool checked) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "binning: %s",
		(checked) ? "YES" : "NO");
	parameters.binning = checked;
}

/**
 * \brief Slot called when the light/dark button is toggled
 */
void	DownloadDialog::lightToggled(bool checked) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "light: %s",
		(checked) ? "YES" : "NO");
	parameters.shutter = checked;
}

/**
 * \brief Slot called when the filter button is toggled
 */
void	DownloadDialog::filterToggled(bool checked) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter: %s",
		(checked) ? "YES" : "NO");
	parameters.filter = checked;
}

/**
 * \brief Slot called when the temperature button is toggled
 */
void	DownloadDialog::temperatureToggled(bool checked) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "temperature: %s",
		(checked) ? "YES" : "NO");
	parameters.temperature = checked;
}

/**
 * \brief Slot called when the dialog is accepted
 */
void	DownloadDialog::acceptedSlot() {
	parameters.prefix = ui->prefixField->text();
}
