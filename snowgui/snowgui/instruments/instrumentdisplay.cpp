/*
 * instrumentdisplay.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "instrumentdisplay.h"
#include "ui_instrumentdisplay.h"

namespace snowgui {

void	instrumentdisplay::toplevel(snowstar::InstrumentComponentType type,
		const std::string& name) {
	// create the header
	QStringList	list;
	list << QString(name.c_str());
	QTreeWidgetItem	*item = new QTreeWidgetItem(list,
		QTreeWidgetItem::Type);
	ui->componentTree->addTopLevelItem(item);
}

void	instrumentdisplay::alltoplevel() {
	toplevel(snowstar::InstrumentAdaptiveOptics, "Adaptive Optics");
	toplevel(snowstar::InstrumentCamera, "Camera");
	toplevel(snowstar::InstrumentCCD, "CCD");
	toplevel(snowstar::InstrumentCooler, "Cooler");
	toplevel(snowstar::InstrumentGuiderCCD, "GuiderCCD");
	toplevel(snowstar::InstrumentGuidePort, "Guideport");
	toplevel(snowstar::InstrumentFilterWheel, "Filterwheel");
	toplevel(snowstar::InstrumentFocuser, "Focuser");
	toplevel(snowstar::InstrumentMount, "Mount");
}

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

static void resizeColumnsToContents( QTreeWidget *treeWidget_ ) {
  int cCols = treeWidget_->columnCount();
  int cItems = treeWidget_->topLevelItemCount();
  int w;
  int col;
  int i;
  for( col = 0; col < cCols; col++ ) {
    w = treeWidget_->header()->sectionSizeHint( col );
    for( i = 0; i < cItems; i++ )
      w = qMax( w, treeWidget_->topLevelItem( i )->text( col ).size()*7 + (col == 0 ? treeWidget_->indentation() : 0) );
    treeWidget_->header()->resizeSection( col, w );
  }
}

void	instrumentdisplay::allchildren() {
	children(snowstar::InstrumentAdaptiveOptics);
	children(snowstar::InstrumentCamera);
	children(snowstar::InstrumentCCD);
	children(snowstar::InstrumentCooler);
	children(snowstar::InstrumentGuiderCCD);
	children(snowstar::InstrumentGuidePort);
	children(snowstar::InstrumentFilterWheel);
	children(snowstar::InstrumentFocuser);
	children(snowstar::InstrumentMount);
	resizeColumnsToContents(ui->componentTree);
}

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
}

instrumentdisplay::~instrumentdisplay() {
	delete ui;
}

void	instrumentdisplay::setInstrument(snowstar::InstrumentPrx instrument) {
	_instrument = instrument;
	allchildren();
}

} // namespace snowgui
