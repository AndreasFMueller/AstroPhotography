/*
 * FitsTable.cpp -- implementation of the Fits table widget
 * 
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <FitsTable.h>
#include <AstroDebug.h>
#include <QHeaderView>
#include <AstroIO.h>

namespace snowgui {

/**
 * \brief Create a FITS table
 */
FitsTable::FitsTable(QWidget *parent) : QTableWidget(parent) {
	QStringList	headerlist;
        headerlist << "Keyword" << "Value" << "Comment";
        setHorizontalHeaderLabels(headerlist);
        horizontalHeader()->setStretchLastSection(true);
}

/**
 * \brief Destroy the FITS table
 */
FitsTable::~FitsTable() {
}

/**
 * \brief Set the data in the FITS table from an image
 *
 * \param image		image to read the parameters from
 */
void	FitsTable::setImage(astro::image::ImagePtr image) {
	QStringList	headerlist;
        headerlist << "Keyword" << "Value" << "Comment";
        setHorizontalHeaderLabels(headerlist);
	QTableWidget	*table = this;
	setRowCount(image->nMetadata() + 3);
	int	row = 0;
	setRowHeight(row, 19);
	setItem(row, 0, new QTableWidgetItem("width"));
	setItem(row, 1, new QTableWidgetItem(astro::stringprintf("%d",
		image->size().width()).c_str()));
	setItem(row, 2, new QTableWidgetItem("width of the image"));
	row++;
	setRowHeight(row, 19);
	setItem(row, 0, new QTableWidgetItem("height"));
	setItem(row, 1, new QTableWidgetItem(astro::stringprintf("%d",
		image->size().height()).c_str()));
	setItem(row, 2, new QTableWidgetItem("height of the image"));
	row++;
	setRowHeight(row, 19);
	setItem(row, 0, new QTableWidgetItem("type"));
	setItem(row, 1, new QTableWidgetItem(
			astro::demangle(image->pixel_type().name()).c_str()));
	setItem(row, 2, new QTableWidgetItem("pixel type of the image"));
	row++;
	for_each(image->begin(), image->end(),
		[table,row](const ImageMetadata::value_type& metadata) mutable {
			table->setRowHeight(row, 19);
			astro::image::Metavalue	v = metadata.second;
			QTableWidgetItem	*i;
			i = new QTableWidgetItem(v.getKeyword().c_str());
			table->setItem(row, 0, i);
			i = new QTableWidgetItem(v.getValue().c_str());
			table->setItem(row, 1, i);
			i = new QTableWidgetItem(v.getComment().c_str());
			table->setItem(row, 2, i);
			row++;
		}
	);
	resizeColumnsToContents();
}

} // namespace snowgui
