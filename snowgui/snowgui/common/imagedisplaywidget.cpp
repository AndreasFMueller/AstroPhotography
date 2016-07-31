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
#include "AutoGain.h"
#include <QScrollBar>

using namespace astro::image;

namespace snowgui {

/**
 * \brief Constructor for the imagedisplaywidget
 */
imagedisplaywidget::imagedisplaywidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::imagedisplaywidget) {
	ui->setupUi(this);

	// connect the imageUpdated signal with the processNewImage slot
	connect(this, SIGNAL(imageUpdated()), this,
		SLOT(processNewImage()), Qt::QueuedConnection);

	// set headers in fits info table
	QStringList	headerlist;
	headerlist << "Keyword" << "Value" << "Comment";
	ui->fitsinfoTable->setHorizontalHeaderLabels(headerlist);
	ui->fitsinfoTable->horizontalHeader()->setStretchLastSection(true);

	// make sure the subframe is disabled
	ui->subframeGroup->setEnabled(false);

	// display the current settings
	displayGainSetting();
	displayBrightnessSetting();
	displayScaleSetting();
}

/**
 * \brief Destructor for the image display widget
 */
imagedisplaywidget::~imagedisplaywidget() {
	delete ui;
}

bool	imagedisplaywidget::settingsIsVisible() {
	return ui->settingsFrame->isVisible();
}

void	imagedisplaywidget::setSettingsVisible(bool h) {
	ui->settingsFrame->setVisible(h);
}

bool    imagedisplaywidget::gainIsVisible() {
	return ui->gainGroup->isVisible();
}

void    imagedisplaywidget::setGainVisible(bool h) {
	ui->gainGroup->setVisible(h);
}

bool    imagedisplaywidget::scaleIsVisible() {
	return ui->scaleGroup->isVisible();
}

void    imagedisplaywidget::setScaleVisible(bool h) {
	ui->scaleGroup->setVisible(h);
}

bool    imagedisplaywidget::subframeIsVisible() {
	return ui->subframeGroup->isVisible();
}

void    imagedisplaywidget::setSubframeVisible(bool h) {
	ui->scaleGroup->setVisible(h);
}

bool	imagedisplaywidget::infoIsVisible() {
	return ui->infoFrame->isVisible();
}

void	imagedisplaywidget::setInfoVisible(bool h) {
	ui->infoFrame->setVisible(h);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new gain setting: %f", g);
	return g;
}

/**
 * \brief read brightness settings
 */
double	imagedisplaywidget::displayBrightnessSetting() {
	double	b = ui->brightnessSlider->value();
	std::string	brightnessstring = astro::stringprintf("%.0f", b);
	ui->brightnessField->setText(QString(brightnessstring.c_str()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new brightness setting: %f", b);
	return b;
}

/**
 * \brief Read display scale settings
 */
int	imagedisplaywidget::displayScaleSetting() {
	int	b = ui->scaleDial->value();
	double	s = 100 * pow(2, b);
	std::string	scalestring = astro::stringprintf("%.0f%%", s);
	ui->scaleField->setText(QString(scalestring.c_str()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new scale setting: %d", b);
	return b;
}

ImageRectangle	imagedisplaywidget::displayWidthSetting() {
	ImageSize	s = _image->size();
	int	newwidth = ui->subframewidthBox->value();
	int	height = ui->subframeheightBox->value();
	int	x = ui->subframexBox->value();
	int	y = ui->subframeyBox->value();
	if (s.width() < x + newwidth) {
		x = s.width() - newwidth;
	}
	ImageRectangle	r(ImageSize(newwidth, height), ImagePoint(x, y));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new rectangle: %s",
		r.toString().c_str());
	return r;
}

ImageRectangle	imagedisplaywidget::displayHeightSetting() {
	ImageSize	s = _image->size();
	int	width = ui->subframewidthBox->value();
	int	newheight = ui->subframeheightBox->value();
	int	x = ui->subframexBox->value();
	int	y = ui->subframeyBox->value();
	if (s.height() < y + newheight) {
		y = s.height() - newheight;
	}
	ImageRectangle	r(ImageSize(width, newheight), ImagePoint(x, y));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new rectangle: %s",
		r.toString().c_str());
	return r;
}

ImageRectangle	imagedisplaywidget::displayXSetting() {
	ImageSize	s = _image->size();
	int	width = ui->subframewidthBox->value();
	int	height = ui->subframeheightBox->value();
	int	newx = ui->subframexBox->value();
	int	y = ui->subframeyBox->value();
	if (newx + width > s.width()) {
		width = s.width() - newx;
	}
	ImageRectangle	r(ImageSize(width, height), ImagePoint(newx, y));
	return r;
}

ImageRectangle	imagedisplaywidget::displayYSetting() {
	ImageSize	s = _image->size();
	int	width = ui->subframewidthBox->value();
	int	height = ui->subframeheightBox->value();
	int	x = ui->subframexBox->value();
	int	newy = ui->subframeyBox->value();
	if (newy + height > s.height()) {
		height = s.height() - newy;
	}
	ImageRectangle	r(ImageSize(width, height), ImagePoint(x, newy));
	return r;
}

void	imagedisplaywidget::displayRectangle(const ImageRectangle& r) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting rectangle %s",
		r.toString().c_str());
	ui->subframewidthBox->blockSignals(true);
	ui->subframewidthBox->setValue(r.size().width());
	ui->subframewidthBox->blockSignals(false);

	ui->subframeheightBox->blockSignals(true);
	ui->subframeheightBox->setValue(r.size().height());
	ui->subframeheightBox->blockSignals(false);

	ui->subframexBox->blockSignals(true);
	ui->subframexBox->setValue(r.origin().x());
	ui->subframexBox->blockSignals(false);

	ui->subframeyBox->blockSignals(true);
	ui->subframeyBox->setValue(r.origin().y());
	ui->subframeyBox->blockSignals(false);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle set");
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
	ui->subframewidthBox->setMaximum(_image->size().width());
	ui->subframeheightBox->setMaximum(_image->size().height());
	ui->subframexBox->setMaximum(_image->size().width());
	ui->subframeyBox->setMaximum(_image->size().height());

	// check whether the current rectangle fits inside the new image
	if (_rectangle.isEmpty()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "empty rectangle, use image size");
		_rectangle = ImageRectangle(_image->size());
	} else {
		if (!_image->size().bounds(_rectangle)) {
			// XXX find a rectangle that works
			_rectangle = ImageRectangle(_image->size());
		}
	}
	displayRectangle(_rectangle);
	ui->subframeGroup->setEnabled(true);

	// retrieve image information and update the info field
	if (_image->hasMetadata("INSTRUME")) {
		std::string	instrument = _image->getMetadata("INSTRUME");
		ui->instrumentField->setText(QString(instrument.c_str()));
	} else {
		ui->instrumentField->setText(QString("(unknown)"));
	}

	// image size and binning
	std::string	sizeinfo = _image->getFrame().toString();
	int	xbin = 0, ybin = 0;
	if (_image->hasMetadata("XBINNING")) {
		xbin = _image->getMetadata("XBINNING");
	}
	if (_image->hasMetadata("YBINNING")) {
		ybin = _image->getMetadata("YBINNING");
	}
	if ((xbin > 0) && (ybin > 0)) {
		sizeinfo = sizeinfo + " / " + Binning(xbin, ybin).toString();
	}
	ui->geometryField->setText(sizeinfo.c_str());

	// pixel type
	std::string	pixeltype
		= astro::demangle(_image->pixel_type().name());
	if (0 == pixeltype.compare(0, 14, "astro::image::")) {
		pixeltype = pixeltype.substr(14);
	}
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

	// query the bayer pattern
	if (_image->hasMetadata("BAYER")) {
		Metavalue	v = _image->getMetadata("EXPTIME");
		ui->bayerField->setText(QString(v.getValue().c_str()));
	} else {
		ui->bayerField->setText(QString("none"));
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
	ui->fitsinfoTable->resizeColumnsToContents();

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

	// get information about the size, we'll need that throughout
	ImageSize	size = _image->size();

	// remember the current position of the scroll area
	int	hpos = ui->imageArea->horizontalScrollBar()->value();
	int	vpos = ui->imageArea->verticalScrollBar()->value();
	QSize	previoussize = ui->imageArea->widget()->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"hpos = %d, vpos = %d, previous size=%d,%d",
		hpos, vpos, previoussize.width(), previoussize.height());

	// create a new label and pixmap
	QLabel  *imageLabel = new QLabel;
	QPixmap *pixmap = image2pixmap(_image);
	if (NULL != pixmap) {
		imageLabel->setPixmap(*pixmap);
	}
	imageLabel->setFixedSize(pixmap->width(), pixmap->height());
	imageLabel->setMinimumSize(pixmap->width(), pixmap->height());

	// display the image
	ui->imageArea->setWidget(imageLabel);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new position: %d/%d", hpos, vpos);
	QSize	newsize = pixmap->size();
	hpos = newsize.width() * hpos / previoussize.width();
	vpos = newsize.height() * vpos / previoussize.height();
	ui->imageArea->horizontalScrollBar()->setValue(hpos);
	ui->imageArea->verticalScrollBar()->setValue(vpos);
	ui->imageArea->show();
	// delete pixmap;

	// update the histogram
	QPixmap *histogram = image2pixmap.histogram(ui->histogramLabel->width(),
		ui->histogramLabel->height());
	if (NULL != histogram) {
		ui->histogramLabel->setPixmap(*histogram);
	}
	// delete histogram;
}

void	imagedisplaywidget::displayAutoGain(const AutoGain& autogain) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "gain=%f, brightness=%f",
		autogain.gain(), autogain.brightness());
	// make sure the gain is in the valid range
	int	gain = 32 * log2(autogain.gain());
	if (gain > 256) {
		gain = 256;
	}
	if (gain < -256) {
		gain = -256;
	}
	//image2pixmap.gain(pow(2, gain / 32));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new gain: %d", gain);

	// make sure the brightness is in the valid range
	int	brightness = autogain.brightness();
	if (brightness > 256) {
		brightness = 256;
	}
	if (brightness < -256) {
		brightness = -256;
	}
	//image2pixmap.brightness(brightness);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new brightness: %d", brightness);

	// set the new values on the slider
	ui->gainSlider->setValue(gain);
	ui->brightnessSlider->setValue(brightness);
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
	if (sender() == ui->autogainButton) {
		if (ui->subframeBox->isChecked()) {
			AutoGain	autogain(_image,
						image2pixmap.rectangle());
			displayAutoGain(autogain);
		} else {
			AutoGain	autogain(_image);
			displayAutoGain(autogain);
		}
	}
	if (sender() == ui->scaleDial) {
		image2pixmap.scale(displayScaleSetting());
	}
	if (sender() == ui->logarithmicBox) {
		image2pixmap.logarithmic(ui->logarithmicBox->isChecked());
	}
	if (sender() == ui->subframewidthBox) {
		_rectangle = displayWidthSetting();
		displayRectangle(_rectangle);
		if (!ui->subframeBox->isChecked()) {
			return;
		}
	}
	if (sender() == ui->subframeheightBox) {
		_rectangle = displayHeightSetting();
		displayRectangle(_rectangle);
		if (!ui->subframeBox->isChecked()) {
			return;
		}
	}
	if (sender() == ui->subframexBox) {
		_rectangle = displayXSetting();
		displayRectangle(_rectangle);
		if (!ui->subframeBox->isChecked()) {
			return;
		}
	}
	if (sender() == ui->subframeyBox) {
		_rectangle = displayYSetting();
		displayRectangle(_rectangle);
		if (!ui->subframeBox->isChecked()) {
			return;
		}
	}
	if (sender() == ui->subframeBox) {
		// nothing needs to be done
	}
	if (sender() == ui->subframefullButton) {
		_rectangle = ImageRectangle(_image->size());
		displayRectangle(_rectangle);
		if (!ui->subframeBox->isChecked()) {
			return;
		}
	}
	if (ui->subframeBox->isChecked()) {
		image2pixmap.rectangle(_rectangle);
	} else {
		image2pixmap.rectangle(ImageRectangle());
	}
	processNewSettings();
}

} // namespace snowgui
