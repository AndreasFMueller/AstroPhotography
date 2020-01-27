/*
 * systemconfigurationwidget.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "systemconfigurationwidget.h"
#include "ui_systemconfigurationwidget.h"

namespace snowgui {

/**
 * \brief Create a SystemConfigurationWidget
 *
 * \param parent	the parent widget
 */
SystemConfigurationWidget::SystemConfigurationWidget(QWidget *parent)
	: QDialog(parent), ui(new Ui::SystemConfigurationWidget) {
	ui->setupUi(this);
}

/**
 * \brief Destroy the widget
 */
SystemConfigurationWidget::~SystemConfigurationWidget() {
	delete ui;
}

/**
 * \brief
 *
 * \param serviceobject		the service object to use for connections
 */
void	SystemConfigurationWidget::setServiceObject(
		astro::discover::ServiceObjectPtr serviceobject) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting ServiceObject in server tab");
	ui->serverTab->setServiceObject(serviceobject);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting ServiceObject in remote tab");
	ui->remoteTab->setServiceObject(serviceobject);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service objects set");
}

/**
 * \brief Slot to handle close events
 */
void	SystemConfigurationWidget::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

} // namespace snowgui
