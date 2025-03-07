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

	// set the various catalog names
	ui->catalogBox->addItem(QString("NGC/IC"));
	ui->catalogBox->addItem(QString("PGC"));
	ui->catalogBox->addItem(QString("Bright Star Catalog"));
	ui->catalogBox->addItem(QString("SAO"));
	ui->catalogBox->addItem(QString("Hipparcos"));
	ui->catalogBox->addItem(QString("Tycho2"));
	ui->catalogBox->addItem(QString("UCAC4"));

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
	connect(ui->catalogBox, SIGNAL(currentIndexChanged(int)),
		this, SLOT(currentItemChanged(int)));

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
 * \brief build the object label string
 *
 * \param name		name of the object
 * \param position	position of the object
 */
std::string	CatalogDialog::labelString(const std::string& name, 
				const astro::RaDec& position) const {
	return astro::stringprintf("%s    @ %s, %s", name.c_str(),
		position.ra().hms(':', 1).c_str(),
		position.dec().dms(':', 0).c_str());
}

/**
 * \brief Common work for the full name search§
 *
 * \param name		name of the object to search for
 */
void	CatalogDialog::searchCommon(const std::string& name) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "searchCommon(\"%s\") currentIndex=%d",
		name.c_str(), ui->catalogBox->currentIndex());
	astro::RaDec	targetpos;
	std::string	targetname;
	try {
		// find out which catalog we are supposed to use
		switch (ui->catalogBox->currentIndex()) {
		case 0: {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"searching NGC for '%s'", name.c_str());
				DeepSkyCatalogFactory	factory;
				DeepSkyCatalogPtr	catalog
					= factory.get(DeepSkyCatalogFactory::NGCIC);
				DeepSkyObject	object = catalog->find(name);
				// update the target
				targetpos = object.position(2000);
				targetname = object.name;
			}
			break;
		case 1: {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"searching PGC for '%s'", name.c_str());
				DeepSkyCatalogFactory	factory;
				DeepSkyCatalogPtr	catalog
					= factory.get(DeepSkyCatalogFactory::PGC);
				DeepSkyObject	object = catalog->find(name);
				// update the target
				targetpos = object.position(2000);
				targetname = object.name;
			}
			break;
		case 2:	{
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"searching BSC for '%s'", name.c_str());
				CatalogFactory	factory;
				CatalogPtr	catalog
					= factory.get(CatalogFactory::BSC);
				Star	object = catalog->find(name);
				targetpos = object.position(2000);
				targetname = object.name();
			}
			break;
		case 3:	{
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"searching SAO for '%s'", name.c_str());
				CatalogFactory	factory;
				CatalogPtr	catalog
					= factory.get(CatalogFactory::SAO);
				Star	object = catalog->find(name);
				targetpos = object.position(2000);
				targetname = object.name();
			}
			break;
		case 4:	{
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"searching Hipparcos for '%s'",
					name.c_str());
				CatalogFactory	factory;
				CatalogPtr	catalog
					= factory.get(CatalogFactory::Hipparcos);
				Star	object = catalog->find(name);
				targetpos = object.position(2000);
				targetname = object.name();
			}
			break;
		case 5:	{
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"searching Tycho2 for '%s'",
					name.c_str());
				CatalogFactory	factory;
				CatalogPtr	catalog
					= factory.get(CatalogFactory::Tycho2);
				Star	object = catalog->find(name);
				targetpos = object.position(2000);
				targetname = object.name();
			}
			break;
		case 6:	{
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"searching UCAC4 for '%s'",
					name.c_str());
				CatalogFactory	factory;
				CatalogPtr	catalog
					= factory.get(CatalogFactory::Ucac4);
				Star	object = catalog->find(name);
				targetpos = object.position(2000);
				targetname = object.name();
			}
			break;
		}
	} catch (const std::exception& x) {
		if (ui->catalogBox->currentIndex() > 0) {
			currentItemChanged(ui->catalogBox->currentIndex());
		} else {
			ui->objectField->setText(QString());
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s not found", name.c_str());
		return;
	}
	std::string	targetlabel = labelString(targetname, targetpos);
	ui->objectField->setText(QString(targetlabel.c_str()));
	emit objectSelected(targetpos);
}

/**
 * \brief Slot called when the user clicks the search button
 *
 * This slot reads the name of the nebula from the search box and then
 * performs a search just as if the user had pressed <return>
 */
void	CatalogDialog::searchClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "looking for object '%s' in catalog %d",
		ui->objectnameField->text().toLatin1().data(),
		ui->catalogBox->currentIndex());
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
 * \brief Handle dso catalog prefix searches
 *
 * \param prefix	prefix of the dso name
 */
void	CatalogDialog::textEditedDSO(const std::string& prefix) {
	// get a the correct catalog
	DeepSkyCatalogFactory	factory;
	DeepSkyCatalogPtr	catalog;
	switch (ui->catalogBox->currentIndex()) {
	case 0:	catalog = factory.get(DeepSkyCatalogFactory::NGCIC);
		break;
	case 1:	catalog = factory.get(DeepSkyCatalogFactory::PGC);
		break;
	}

	// retrieve names
	std::set<std::string>	names = catalog->findLike(prefix);
	if (names.size() == 0) {
		return;
	}

	// prepare to fill the list with object names
	QFont	font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	ui->listWidget->setFont(font);
	for (auto i = names.begin(); i != names.end(); i++) {
		DeepSkyObject	dso = catalog->find(*i);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d matching names",
		names.size());
}

/**
 * \brief Handle star catalog prefix searches
 *
 * \param prefix	prefix of the star name
 */
void	CatalogDialog::textEditedStars(const std::string& prefix) {
	// handle star catalogs
	astro::catalog::Catalog::starsetptr	stars;
	try {
		astro::catalog::CatalogFactory	factory;
		astro::catalog::CatalogPtr	catalog;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting stars for prefix %s",
			prefix.c_str());
		switch (ui->catalogBox->currentIndex()) {
		case 0:
		case 1:
			break;
		case 2:	catalog = factory.get(CatalogFactory::BSC);
			break;
		case 3:	catalog = factory.get(CatalogFactory::SAO);
			break;
		case 4:	catalog = factory.get(CatalogFactory::Hipparcos);
			break;
		case 5:	catalog = factory.get(CatalogFactory::Tycho2);
			break;
		case 6:	catalog = factory.get(CatalogFactory::Ucac4);
			break;
		}
		stars = catalog->findLike(prefix);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot get stars from catalog %d: %s",
				ui->catalogBox->currentIndex(), x.what());
		return;
	}

	// make sure we have some stars
	if (!stars) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no stars returned");
		return;
	}
	if (stars->size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no stars returned");
		return;
	}

	// get the font
	QFont	font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
	ui->listWidget->setFont(font);

	// list the star entries
	std::set<std::string>	starstrings = Catalog::starlist(stars);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d star strings", starstrings.size());
	QListWidget	*listWidget = ui->listWidget;
	std::for_each(starstrings.begin(), starstrings.end(),
		[listWidget,font](const std::string& s) mutable {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %s", s.c_str());
			QListWidgetItem	*item = new QListWidgetItem(
				QString(s.c_str()));
			item->setFont(font);
			listWidget->addItem(item);
		}
	);
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

	// perform NGC catalog search
	if (ui->catalogBox->currentIndex() < 2) {
		textEditedDSO(prefix);
	} else {
		textEditedStars(prefix);
	}
	return;
#if 0
	// handle star catalogs
	
	astro::catalog::Catalog::starsetptr	stars;
	try {
		astro::catalog::CatalogFactory	factory;
		astro::catalog::CatalogPtr	catalog;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "getting stars for prefix %s",
			prefix.c_str());
		switch (ui->catalogBox->currentIndex()) {
		case 0:
		case 1:
			break;
		case 2:	catalog = factory.get(CatalogFactory::BSC);
			break;
		case 3:	catalog = factory.get(CatalogFactory::Hipparcos);
			break;
		case 4:	catalog = factory.get(CatalogFactory::Tycho2);
			break;
		case 5:	catalog = factory.get(CatalogFactory::Ucac4);
			break;
		}
		stars = catalog->findLike(prefix);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"cannot get stars from catalog %d: %s",
				ui->catalogBox->currentIndex(), x.what());
		return;
	}

	if (stars) {
		std::set<std::string>	starstrings = Catalog::starlist(stars);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d star strings",
			starstrings.size());
		QListWidget	*listWidget = ui->listWidget;
		std::for_each(starstrings.begin(), starstrings.end(),
			[listWidget,font](const std::string& s) mutable {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "adding %s",
					s.c_str());
				QListWidgetItem	*item = new QListWidgetItem(
					QString(s.c_str()));
				item->setFont(font);
				listWidget->addItem(item);
			}
		);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no stars returned");
	}
#endif
}

/**
 * \brief change the selected catalog
 *
 * \param index		new current index
 */
void	CatalogDialog::currentItemChanged(int index) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new catalog selection: %d", index);
	switch (index) {
	case 0:
		ui->objectField->setText("<font color='white'>NGC1234 or IC1234</font>");
		break;
	case 1:
		ui->objectField->setText("<font color='white'>PGC0002557</font>");
		break;
	case 2:
		ui->objectField->setText("<font color='white'>BSC12345</font>");
		break;
	case 3:
		ui->objectField->setText("<font color='white'>SAO123456</font>");
		break;
	case 4:
		ui->objectField->setText("<font color='white'>HIP123456</font>");
		break;
	case 5:
		ui->objectField->setText("<font color='white'>T1234 12345 1</font>");
		break;
	case 6:
		ui->objectField->setText("<font color='white'>UCAC4-123-123456</font>");
		break;
	}
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found object named %s", s.c_str());
	searchChanged(QString(s.c_str()));
}

/**
 * \brief Handle destruction of the dialog
 */
void	CatalogDialog::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

} // namespace snowgui
