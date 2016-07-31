/*
 * imagedisplaywidget.cpp -- FITS image display implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "imagedisplaywidget.h"
#include "ui_imagedisplaywidget.h"
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroImage.h>
#include <AstroFilterfunc.h>

using namespace astro::image;

/**
 * \brief Constructor for the imagedisplaywidget
 */
imagedisplaywidget::imagedisplaywidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::imagedisplaywidget)
{
	ui->setupUi(this);

	// connect the imageUpdated signal with the processNewImage slot
	connect(this, SIGNAL(imageUpdated()), this,
		SLOT(processNewImage()), Qt::QueuedConnection);
}

/**
 * \brief Destructor for the image display widget
 */
imagedisplaywidget::~imagedisplaywidget()
{
    delete ui;
}

/**
 * \brief read gain settings
 */
double	imagedisplaywidget::displayGainSetting() {
	double  v = ui->gainSlider->value();
	double  g = pow(2, (double)v / 32.);
	std::string     gainstring;
	if (g >= 1) {
		gainstring = astro::stringprintf("%.1f", g);
	} else {
		gainstring = astro::stringprintf("1/%.1f", 1/g);
	}
	ui->gainField->setText(QString(gainstring.c_str()));
	return g;
}

/**
 * \brief read brightness settings
 */
double	imagedisplaywidget::displayBrightnessSetting() {
	double	b = ui->brightnessSlider->value();
	std::string	brightnessstring = astro::stringprintf("%.0f", b);
	ui->brightnessField->setText(QString(brightnessstring.c_str()));
	return b;
}

/**
 * \brief Read display scale settings
 */
double	imagedisplaywidget::displayScaleSetting() {
	double	b = pow(2, ui->scaleDial->value());
	double	s = 100 * b;
	std::string	scalestring = astro::stringprintf("%.0f%%", s);
	ui->scaleField->setText(QString(scalestring.c_str()));
	return b;
}

/**
 * \brief set the new image
 *
 * This method just remembers the new image and emits the imageUpdated
 * signal. The main thread will then execute the 
 */
void	imagedisplaywidget::setImage(astro::image::ImagePtr image) {
	_image = image;
	emit imageUpdated();
}

/**
 * \brief Processing done for a new image
 */
void	imagedisplaywidget::processNewImage() {
	if (!_image) {
		return;
	}
	// retrieve image information and update the info field
	ui->geometryField->setText(_image->getFrame().toString().c_str());

	// pixel type
	std::string	pixeltype
		= astro::demangle(_image->pixel_type().name());
	ui->pixeltypeField->setText(QString(pixeltype.c_str()));

	// read pixel value statistics
	double	maximum = astro::image::filter::max(_image);
	double	minimum = astro::image::filter::min(_image);
	double	mean = astro::image::filter::mean(_image);
	std::string	minmax = astro::stringprintf("%f/%f/%f",
		minimum, mean, maximum);
	ui->minmaxField->setText(QString(minmax.c_str()));

	// query exposure time
	if (_image->hasMetadata("EXPTIME")) {
		Metavalue	v = _image->getMetadata("EXPTIME");
		ui->exposuretimeField->setText(QString(v.getValue().c_str()));
	} else {
		ui->exposuretimeField->setText(QString("unknown"));
	}

	// read meta data from the image and display in the FITS info area
	QTableWidget	*table = ui->fitsinfoTable;
	table->setRowCount(_image->nMetadata());
	int	row = 0;
	for_each(_image->begin(), _image->end(),
		[table,row](const ImageMetadata::value_type& metadata) mutable {
			Metavalue	v = metadata.second;
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

	// do the processing that depends on the settings
	processNewSettings();
}

/**
 * \brief process new image settings
 *
 * This slot is called to retrieve the new settings and to reprocess the
 * image for display.
 */
void	imagedisplaywidget::processNewSettings() {
	// if there is no image, we don't need to do anything
	if (!_image) {
		return;
	}
}

/**
 * \brief Read modified settings and initiate reprocessing of the image
 *
 * This slot checks from which ui object the change came and updates
 * the corresponding display element (for gain, brightness and scale).
 * It then calls the processNewSettings slot to ensure that the image
 * display is updated;
 */
void	imagedisplaywidget::imageSettingsChanged() {
	// check which settings have changed
	if (sender() == ui->gainSlider) {
		image2pixmap.gain(displayGainSetting());
	}
	if (sender() == ui->brightnessSlider) {
		image2pixmap.brightness(displayBrightnessSetting());
	}
	if (sender() == ui->scaleDial) {
		image2pixmap.scale(displayScaleSetting());
	}
	if (sender() == ui->logarithmicBox) {
		image2pixmap.logarithmic(ui->logarithmicBox->isChecked());
	}
	processNewSettings();
}
