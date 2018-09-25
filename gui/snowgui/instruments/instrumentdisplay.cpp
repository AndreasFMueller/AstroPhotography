/*
 * instrumentdisplay.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "instrumentdisplay.h"
#include "ui_instrumentdisplay.h"
#include <AstroDevice.h>

namespace snowgui {

/**
 * \brief Construct an instrument display
 */
instrumentdisplay::instrumentdisplay(QWidget *parent)
	: QWidget(parent), ui(new Ui::instrumentdisplay) {
	ui->setupUi(this);

	// headers for the component table
	QStringList	componentheaders;
	componentheaders << "Name" << "Index" << "Server";
	ui->componentTree->setHeaderLabels(componentheaders);

	// headers for the property table
	QStringList	headerlist;
	headerlist << "Property" << "Value" << "Description";
	ui->propertyTable->setHorizontalHeaderLabels(headerlist);

	// create the top level items
	alltoplevel();

	// value change in table
	connect(ui->propertyTable, SIGNAL(cellChanged(int,int)),
		this, SLOT(propertyValueChanged(int,int)));
}

/**
 * \brief Destroy the instrument display
 */
instrumentdisplay::~instrumentdisplay() {
	delete ui;
}

/**
 * \brief add a top level tree entry of a given type
 */
void	instrumentdisplay::toplevel(snowstar::InstrumentComponentType /* type */,
		const std::string& name) {
	// create the header
	QStringList	list;
	list << QString(name.c_str());
	QTreeWidgetItem	*item = new QTreeWidgetItem(list,
		QTreeWidgetItem::Type);
	ui->componentTree->addTopLevelItem(item);
    	ui->componentTree->header()->resizeSection(1, 50);
    	ui->componentTree->header()->resizeSection(2, 100);
    	ui->componentTree->header()->resizeSection(0, 300);
}

/**
 * \brief Add and display all the toplevel entries in the tree
 */
void	instrumentdisplay::alltoplevel() {
	toplevel(snowstar::InstrumentAdaptiveOptics, "Adaptive Optics");
	toplevel(snowstar::InstrumentCamera, "Camera");
	toplevel(snowstar::InstrumentCCD, "CCD");
	toplevel(snowstar::InstrumentCooler, "Cooler");
	toplevel(snowstar::InstrumentGuiderCCD, "GuiderCCD");
	toplevel(snowstar::InstrumentGuiderCCD, "FinderCCD");
	toplevel(snowstar::InstrumentGuidePort, "Guideport");
	toplevel(snowstar::InstrumentFilterWheel, "Filterwheel");
	toplevel(snowstar::InstrumentFocuser, "Focuser");
	toplevel(snowstar::InstrumentMount, "Mount");
}

/**
 * \brief add device children of a given type
 */
void	instrumentdisplay::children(snowstar::InstrumentComponentType type) {
	// give up if we have no instrument
	if (!_instrument) {
		return;
	}

	// remove all children from the top level widget
	QTreeWidgetItem	*top = ui->componentTree->topLevelItem((int)type);
	while (top->childCount() > 0) {
		top->removeChild(top->child(0));
	}

	// now retreive components of this type
	int	n = _instrument->nComponentsOfType(type);

	for (int index = 0; index < n; index++) {
		snowstar::InstrumentComponent	component
			= _instrument->getComponent(type, index);
		QStringList	list;
		list << QString(component.deviceurl.c_str());
		list << QString::number(component.index);
		list << QString(component.servicename.c_str());
		QTreeWidgetItem	*componentitem = new QTreeWidgetItem(list,
			QTreeWidgetItem::Type);
		top->addChild(componentitem);
	}
	top->setExpanded(true);
}

/**
 * \brief add all device children
 */
void	instrumentdisplay::allchildren() {
	children(snowstar::InstrumentAdaptiveOptics);
	children(snowstar::InstrumentCamera);
	children(snowstar::InstrumentCCD);
	children(snowstar::InstrumentCooler);
	children(snowstar::InstrumentGuiderCCD);
	children(snowstar::InstrumentFinderCCD);
	children(snowstar::InstrumentGuidePort);
	children(snowstar::InstrumentFilterWheel);
	children(snowstar::InstrumentFocuser);
	children(snowstar::InstrumentMount);
}

/**
 * \brief choose an instrument
 */
void	instrumentdisplay::setInstrument(snowstar::InstrumentPrx instrument) {
	_instrument = instrument;
	allchildren();
	allproperties();
}

/**
 * \brief Convert a device name into an instrument component type
 */
static snowstar::InstrumentComponentType	convert(const astro::DeviceName& d) {
	switch (d.type()) {
	case astro::DeviceName::AdaptiveOptics:
		return  snowstar::InstrumentAdaptiveOptics;
	case astro::DeviceName::Camera:
		return  snowstar::InstrumentCamera;
	case astro::DeviceName::Ccd:
		return  snowstar::InstrumentCCD;
	case astro::DeviceName::Cooler:
		return  snowstar::InstrumentCooler;
	case astro::DeviceName::Filterwheel:
		return  snowstar::InstrumentFilterWheel;
	case astro::DeviceName::Focuser:
		return  snowstar::InstrumentFocuser;
	case astro::DeviceName::Guideport:
		return  snowstar::InstrumentGuidePort;
	case astro::DeviceName::Mount:
		return  snowstar::InstrumentMount;
	default:
		break;
	}
	throw std::runtime_error("no associated instrument type");
}

/**
 * \brief Add a device to the instrument
 */
void	instrumentdisplay::add(const std::string& devicename,
		const std::string& servicename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add device %s to service %s",
		devicename.c_str(), servicename.c_str());
	if (!_instrument) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no instrument");
		return;
	}
	snowstar::InstrumentComponent	component;
	component.instrumentname = _instrument->name();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "device name is %s",
		component.instrumentname.c_str());
	component.deviceurl = devicename;
	component.servicename = servicename;
	astro::DeviceName	d(devicename);
	component.type = convert(d);
	component.index = _instrument->nComponentsOfType(component.type);
	_instrument->add(component);

	// now make sure the list is redisplayed
	redisplay();
}

/**
 * \brief Add a CCD device as a Guider CCD
 */
void	instrumentdisplay::addGuiderCCD(const std::string& devicename,
		const std::string& servicename) {
	if (!_instrument) {
		return;
	}
	snowstar::InstrumentComponent	component;
	component.instrumentname = _instrument->name();
	component.deviceurl = devicename;
	component.servicename = servicename;
	astro::DeviceName	d(devicename);
	component.type = snowstar::InstrumentGuiderCCD;
	if (convert(d) != snowstar::InstrumentCCD) {
		return;
	}
	component.index = _instrument->nComponentsOfType(component.type);
	_instrument->add(component);

	redisplay();
}

/**
 * \brief Add a CCD device as a Finder CCD
 */
void	instrumentdisplay::addFinderCCD(const std::string& devicename,
		const std::string& servicename) {
	if (!_instrument) {
		return;
	}
	snowstar::InstrumentComponent	component;
	component.instrumentname = _instrument->name();
	component.deviceurl = devicename;
	component.servicename = servicename;
	astro::DeviceName	d(devicename);
	component.type = snowstar::InstrumentFinderCCD;
	if (convert(d) != snowstar::InstrumentCCD) {
		return;
	}
	component.index = _instrument->nComponentsOfType(component.type);
	_instrument->add(component);

	redisplay();
}

/**
 * \brief Delete the selected component from the instrument
 */
void	instrumentdisplay::deleteSelected() {
	if (!_instrument) {
		return;
	}
	QList<QTreeWidgetItem *>	selected = ui->componentTree->selectedItems();
	if (selected.count() == 0) {
		return;
	}
	QTreeWidgetItem	*item = selected.first();
	// find the parent widget
	if (ui->componentTree->invisibleRootItem() == item->parent()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot delete top level items");
		return;
	}

	// find out whether this is a guiderport item
	bool	isGuiderCCD = (item->parent()->text(0) == QString("GuiderCCD"));
	bool	isFinderCCD = (item->parent()->text(0) == QString("FinderCCD"));

	// read name, type and index from the item
	snowstar::InstrumentComponentType	type;
	try {
		type = convert(astro::DeviceName(std::string(item->text(0).toLatin1().data())));
	} catch (std::exception& x) {
		return;
	}
	int	index = item->text(1).toInt();
	if (isGuiderCCD) {
		type = snowstar::InstrumentGuiderCCD;
	}
	if (isFinderCCD) {
		type = snowstar::InstrumentFinderCCD;
	}
	try {
		_instrument->remove(type, index);
	} catch (const snowstar::NotFound& x) {
		// inform user that device was not found
	}
	redisplay();
}

/**
 * \brief redisplay the component tree
 */
void	instrumentdisplay::redisplay() {
	allchildren();
}

/**
 * \brief build the contents of the property tree
 */
void	instrumentdisplay::allproperties() {
	if (!_instrument) {
		return;
	}
	ui->propertyTable->blockSignals(true);
	property(0, "focallength", "Focal length [m] of main camera");
	property(1, "imagerazimuth", "Azimuth of imager ccd [degrees]");
	property(2, "guiderfocallength", "Focal length [m] of guide camera");
	property(3, "guiderazimuth", "Azimuth of guider ccd [degrees]");
	property(4, "guiderate", "mount rate wrt. siderial rate");
	property(5, "finderfocallength", "Focal length [m] of finder");
	property(6, "finderazimuth", "Azimuth of finder ccd [degrees]");
	ui->propertyTable->blockSignals(false);
}

/**
 * \brief build the contents of a single property
 */
void    instrumentdisplay::property(int row, const std::string& propertyname, 
                        const std::string& description) {
	snowstar::InstrumentProperty	p;
	try {
		p = _instrument->getProperty(propertyname);
	} catch (const std::exception& x) {
		p.instrumentname = _instrument->name();
		p.property = propertyname;
		p.value = std::string();
		p.description = description;
	}
	ui->propertyTable->setRowHeight(row, 19);
	QTableWidgetItem	*i;
	i = new QTableWidgetItem(p.property.c_str());
	i->setFlags(Qt::NoItemFlags);
        ui->propertyTable->setItem(row, 0, i);

	i = new QTableWidgetItem(p.value.c_str());
	i->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        ui->propertyTable->setItem(row, 1, i);

	i = new QTableWidgetItem(p.description.c_str());
	i->setFlags(Qt::NoItemFlags);
        ui->propertyTable->setItem(row, 2, i);

	ui->propertyTable->resizeColumnsToContents();
}

void	instrumentdisplay::propertyValueChanged(int row, int /* column */) {
	snowstar::InstrumentProperty	p;
	p.instrumentname = _instrument->name();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "property in row %d value changed", row);
	std::string	name;
	switch (row) {
	case 0:	name = "focallength"; break;
	case 1:	name = "azimuth"; break;
	case 2:	name = "guiderfocallength"; break;
	case 3:	name = "guiderazimuth"; break;
	case 4:	name = "guiderate"; break;
	case 5: name = "finderfocallength"; break;
	case 6: name = "finderazimuth"; break;
	}
	p.property = name;
	QTableWidgetItem	*item = ui->propertyTable->item(row, 1);
	p.value = std::string(item->text().toLatin1().data());
	item = ui->propertyTable->item(row, 2);
	p.description = std::string(item->text().toLatin1().data());
	try {
		_instrument->getProperty(name);
		_instrument->updateProperty(p);
	} catch (const std::exception& x) {
		_instrument->addProperty(p);
	}
}

} // namespace snowgui
