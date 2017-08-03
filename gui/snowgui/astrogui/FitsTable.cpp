/*
 * FitsTable.cpp -- implementation of the Fits table widget
 * 
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <FitsTable.h>
#include <AstroDebug.h>
#include <QHeaderView>
#include <AstroIO.h>
#include <AstroFilterfunc.h>

namespace snowgui {

/**
 * \brief Create a FITS table
 */
FitsTable::FitsTable(QWidget *parent) : QTableWidget(parent) {
        horizontalHeader()->setStretchLastSection(true);
	_synthetic = true;
}

/**
 * \brief Destroy the FITS table
 */
FitsTable::~FitsTable() {
}

static void	metaItem(int& row, QTableWidget* table,
			const std::string& keyword,
			const std::string& value,
			const std::string& comment) {
	table->setRowHeight(row, 19);
	QTableWidgetItem	*i;
	i = new QTableWidgetItem(keyword.c_str());
	table->setItem(row, 0, i);
	i = new QTableWidgetItem(value.c_str());
	table->setItem(row, 1, i);
	i = new QTableWidgetItem(comment.c_str());
	table->setItem(row, 2, i);
	row++;
}

/**
 * \brief Set the data in the FITS table from an image
 *
 * \param image		image to read the parameters from
 */
void	FitsTable::setImage(astro::image::ImagePtr image) {
	QStringList	headerlist;
        headerlist << "Keyword" << "Value" << "Comment";
        this->setHorizontalHeaderLabels(headerlist);
        this->horizontalHeader()->setStretchLastSection(true);
	QTableWidget	*table = this;
	setRowCount(image->nMetadata() + ((synthetic()) ? 3 : 0));
	int	row = 0;
	if (synthetic()) {
		// width
		metaItem(row, this, "width", astro::stringprintf("%d",
			image->size().width()), "width of the image");
		// height
		metaItem(row, this, "height", astro::stringprintf("%d",
			image->size().height()), "height of the image");
		// pixel type
		metaItem(row, this, "type", 
			astro::demangle(image->pixel_type().name()),
			"pixel type of image");

		// read pixel value statistics
		double  maximum = 0;
		double  minimum = 0;
		double  mean = 0;
		if (3 == image->planes()) {
			maximum = astro::image::filter::max_luminance(image);
			minimum = astro::image::filter::min_luminance(image);
			mean = astro::image::filter::mean_luminance(image);
		} else {
			maximum = astro::image::filter::max(image);
			minimum = astro::image::filter::min(image);
			mean = astro::image::filter::mean(image);
		}
		metaItem(row, this, "minimum",
			astro::stringprintf("%.3f", minimum),
			"minimum pixel value");
		metaItem(row, this, "mean",
			astro::stringprintf("%.3f", mean),
			"mean pixel value");
		metaItem(row, this, "maximum",
			astro::stringprintf("%.3f", maximum),
			"maximum pixel value");
	}
	for_each(image->begin(), image->end(),
		[table,row](const ImageMetadata::value_type& metadata) mutable {
			astro::image::Metavalue	v = metadata.second;
			metaItem(row, table, 
				v.getKeyword().c_str(),
				v.getValue().c_str(),
				v.getComment().c_str());
		}
	);
	resizeColumnsToContents();
}

} // namespace snowgui
