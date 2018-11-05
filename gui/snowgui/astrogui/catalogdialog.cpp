/*
 * catalogdialog.cpp -- dialog to select an object from the catalog
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "catalogdialog.h"
#include "ui_catalogdialog.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <QFontDatabase>

using namespace astro::catalog;

namespace snowgui {

/**
 * \brief Construct a new catalog widget
 *
 * \param parent 	parent widget
 */
CatalogDialog::CatalogDialog(QWidget *parent)
	: QDialog(parent), ui(new Ui::CatalogDialog) {
	ui->setupUi(this);

	setWindowTitle(QString("Search deep sky catalog"));

	QFont	font("Microsoft Sans Serif");
	font.setStyleHint(QFont::TypeWriter);
	ui->listWidget->setFont(font);

	connect(ui->searchButton, SIGNAL(clicked()),
		this, SLOT(searchClicked()));
	connect(ui->objectnameField, SIGNAL(returnPressed()),
		this, SLOT(searchClicked()));
	connect(ui->objectnameField, SIGNAL(textChanged(const QString&)),
		this, SLOT(textEdited(const QString&)));
	connect(ui->listWidget, SIGNAL(itemActivated(QListWidgetItem*)),
		this, SLOT(nameActivated(QListWidgetItem*)));

	DeepSkyCatalogFactory	factory;
	_catalog = factory.get(DeepSkyCatalogFactory::NGCIC);
}

/**
 * \brief Destroy the catalog dialog
 */
CatalogDialog::~CatalogDialog() {
	delete ui;
}

/**
 * \brief Common work for the full name search§
 *
 * \param name		name of the object to search for
 */
void	CatalogDialog::searchCommon(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "searchCommon(\"%s\")", name.c_str());
	try {
		DeepSkyObject	object = _catalog->find(name);
		// update the target
		astro::RaDec	target = object.position(2000);
		emit objectSelected(target);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s not found", name.c_str());
	}
}

/**
 * \brief Slot called when the user clicks the search button
 *
 * This slot reads the name of the nebula from the search box and then
 * performs a search just as if the user had pressed <return>
 */
void	CatalogDialog::searchClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for object '%s'",
		ui->objectnameField->text().toLatin1().data());
	std::string	name(ui->objectnameField->text().toLatin1().data());
	searchCommon(name);
}

/**
 * \brief Slot called when the text in the search box is finalized
 *
 * By finalized whe mean that the user pressed <return> and thus meant
 * to select that object.
 */
void	CatalogDialog::searchChanged(const QString& newtext) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "search for %s",
		newtext.toLatin1().data());
	std::string	name(newtext.toLatin1().data());
	searchCommon(name);
}

/**
 * \brief Slot called when the text in the search box changes
 *
 * This method considers the text in the search box a prefix and searches
 * all matching nebulae in the catalog and displays them in the list widget.
 *
 * \param newtext	prefix to search for in the catalog
 */
void	CatalogDialog::textEdited(const QString& newtext) {
	std::string	prefix;
	for (char *d = newtext.toLatin1().data(); *d; d++) {
		if (*d != ' ') {
			prefix.push_back(*d);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "search for prefix %s", prefix.c_str());
	ui->listWidget->clear();
	QFont	font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	ui->listWidget->setFont(font);
	std::set<std::string>	names = _catalog->findLike(prefix);
	for (auto i = names.begin(); i != names.end(); i++) {
		DeepSkyObject	dso = _catalog->find(*i);
		astro::RaDec	rd = dso.position(2000);
		std::string	l = astro::stringprintf("%-20.20s|  %s %s  |  %s",
			i->c_str(),
			rd.ra().hms(':', 1).substr(1).c_str(),
			rd.dec().dms(':', 0).c_str(),
			DeepSkyObject::classification2string(dso.classification).c_str());

		QListWidgetItem	*item = new QListWidgetItem(QString(l.c_str()));
		item->setFont(font);
		ui->listWidget->addItem(item);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d matching names", names.size());
}

/**
 * \brief Slot called when an item in the list is double clicked
 *
 * When a user double clicks on an item, it is retrieved from the catalog
 * and its data is sent to the mount controller widget for a search.
 *
 * \param item		list item that contains the name of the object
 */
void	CatalogDialog::nameActivated(QListWidgetItem *item) {
	// get the name from thje widget and use the searchChanged slot
	std::string	s(item->text().toLatin1().data());
	s = s.substr(0, s.find('|'));
	s = astro::trim(s);
	searchChanged(QString(s.c_str()));
}

/**
 * \brief Handle destruction of the dialog
 */
void	CatalogDialog::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

} // namespace snowgui
