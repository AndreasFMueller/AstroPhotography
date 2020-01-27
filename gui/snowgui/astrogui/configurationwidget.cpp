/*
 * configurationwidget.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "configurationwidget.h"
#include "ui_configurationwidget.h"

#include <AstroDebug.h>
#include <QTableWidget>
#include <QTableWidgetItem>

namespace snowgui {

/**
 * \brief Construct a configurationwidget
 */
configurationwidget::configurationwidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::configurationwidget) {
	ui->setupUi(this);

	// table configuration
	ui->configTable->horizontalHeader()->setStretchLastSection(true);

	// connect buttons to slots
	connect(ui->deleteButton, SIGNAL(clicked()),
		this, SLOT(deleteClicked()));
	connect(ui->refreshButton, SIGNAL(clicked()),
		this, SLOT(refreshClicked()));
	connect(ui->configTable, SIGNAL(cellChanged(int, int)),
		this, SLOT(valueChanged(int, int)));
	connect(ui->configTable, SIGNAL(itemSelectionChanged()),
		this, SLOT(selectionChanged()));

	// disalbe the delete button
	ui->deleteButton->setEnabled(false);
}

/**
 * \brief Destroy the configuration widget
 */
configurationwidget::~configurationwidget() {
	delete ui;
}

void	configurationwidget::closeEvent(QCloseEvent* /* event */) {
	deleteLater();
}

/**
 * \brief Extract the key for a row from the table widget
 */
astro::config::ConfigurationKey	configurationwidget::key(int row) {
	QTableWidgetItem	*item = NULL;
	item = ui->configTable->item(row, 0);
	std::string	domain(	item->text().toLatin1().data());
	item = ui->configTable->item(row, 1);
	std::string	section(item->text().toLatin1().data());
	item = ui->configTable->item(row, 2);
	std::string	name(	item->text().toLatin1().data());
	return astro::config::ConfigurationKey(domain, section, name);
}

/**
 * \brief Extract the key for a row containing a widget
 */
astro::config::ConfigurationKey	configurationwidget::key(QTableWidgetItem *item) {
	int	row = ui->configTable->row(item);
	return key(row);
}

/**
 * \brief Slot for refresh button
 */
void	configurationwidget::refreshClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "refresh clicked");
	filltable();
}

/**
 * \brief Slot for delete button
 */
void	configurationwidget::deleteClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete clicked");
	ui->configTable->blockSignals(true);

	// find the selected item
	QList<QTableWidgetItem*>	list = ui->configTable->selectedItems();

	for (auto li = list.begin(); li != list.end(); li++) {
		// remove the key
		remove(key(*li));

		// remove the item from the database
		(*li)->setText(QString());
	}

	ui->configTable->blockSignals(false);
}

/**
 * \brief Retrieve a list of known keys
 */
std::list<astro::config::ConfigurationKey>	configurationwidget::listkeys() {
	return astro::config::Configuration::listRegistered();
}

/**
 * \brief Find out whether the Configuration actually has a certain key
 *
 * \param key	the key to test
 */
bool	configurationwidget::has(const astro::config::ConfigurationKey& key) {
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	return config->has(key);
}

/**
 * \brief Get the description associated with this key
 *
 * \param key	the key to test
 */
std::string	configurationwidget::description(
			const astro::config::ConfigurationKey& key) {
	return astro::config::Configuration::describe(key);
}

/**
 * \brief
 *
 * \param key	the key to test
 */
std::string	configurationwidget::value(
			const astro::config::ConfigurationKey& key) {
	try {
		astro::config::ConfigurationPtr	config
			= astro::config::Configuration::get();
		return config->get(key);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "key '%s' does not exist",
			key.toString().c_str());
		throw x;
	}
}

/**
 * \brief Set a configuration value
 *
 * \param key		the configuration key
 * \param value		the value ot associate with this key
 */
void	configurationwidget::set(
			const astro::config::ConfigurationKey& key,
			const std::string& value) {
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	config->set(key, value);
}

/**
 * \brief Remove an entry from the configuration
 *
 * \param key		the configuration key to remove
 */
void	configurationwidget::remove(
			const astro::config::ConfigurationKey& key) {
	astro::config::ConfigurationPtr	config
		= astro::config::Configuration::get();
	config->remove(key);
}

/**
 * \brief Fill the table
 */
void	configurationwidget::filltable() {
	ui->configTable->blockSignals(true);

	// get a list of keas
	auto	l = this->listkeys();
	ui->configTable->setRowCount(l.size());

	// make sure we have the right number of 
	int	row = 0;
	for (auto k = l.begin(); k != l.end(); k++, row++) {
		QTableWidgetItem	*i;

		i = new QTableWidgetItem(k->domain().c_str());
		i->setFlags(Qt::NoItemFlags);
		ui->configTable->setItem(row, 0, i);

		i = new QTableWidgetItem(k->section().c_str());
		i->setFlags(Qt::NoItemFlags);
		ui->configTable->setItem(row, 1, i);

		i = new QTableWidgetItem(k->name().c_str());
		i->setFlags(Qt::NoItemFlags);
		ui->configTable->setItem(row, 2, i);

		if (has(*k)) {
			std::string	v = value(*k);
			i = new QTableWidgetItem(v.c_str());
		} else {
			i = new QTableWidgetItem("");
		}
		i->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable
			| Qt::ItemIsEnabled);
		ui->configTable->setItem(row, 3, i);

		i = new QTableWidgetItem(description(*k).c_str());
		i->setFlags(Qt::NoItemFlags);
		ui->configTable->setItem(row, 4, i);
	}

	ui->configTable->resizeColumnsToContents();

	ui->configTable->blockSignals(false);
}

/**
 * \brief Read the new configuration value and store it
 *
 * \param row		the row of the changed value
 * \param column	the column of the changee value
 */
void	configurationwidget::valueChanged(int row, int column) {
	if (column != 3) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "row %d has changed");
	QTableWidgetItem	*item = NULL;

	// retrieve the key from the table
	item = ui->configTable->item(row, 0);
	std::string	domain(	item->text().toLatin1().data());
	item = ui->configTable->item(row, 1);
	std::string	section(item->text().toLatin1().data());
	item = ui->configTable->item(row, 2);
	std::string	name(	item->text().toLatin1().data());
	astro::config::ConfigurationKey	key(domain, section, name);

	// get the value
	item = ui->configTable->item(row, 3);
	std::string	value = astro::trim(
				std::string(item->text().toLatin1().data()));

	// if the value is empty, remove the key
	if (0 == value.size()) {
		remove(key);
	} else {
		// set the key
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set %s -> %s",
			key.toString().c_str(), value.c_str());
		set(key, value);
	}
}

/**
 * \brief Decide whether the delete button should be enabled
 */
void	configurationwidget::selectionChanged() {
	QList<QTableWidgetItem*>	list = ui->configTable->selectedItems();
	ui->deleteButton->setEnabled(list.size() > 0);
}

} // namespace snowgui
