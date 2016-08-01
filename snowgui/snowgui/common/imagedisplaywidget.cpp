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

	// make sure the subframe is disabled, it only becomes enabled
	// when an image is added
	ui->subframeGroup->setEnabled(false);

	// display the current settings
	displayGainSetting();
	displayBrightnessSetting();
	displayScaleSetting();

	// initialize
	selectable = NULL;
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
	// if the info was previously invisble, we have to display it now
	if ((!ui->infoFrame->isVisible()) && h) {
		processNewImageInfo(_image);
	}
	ui->infoFrame->setVisible(h);
}

ImageRectangle	imagedisplaywidget::imageRectangle() {
	return _rectangle;
}

/**
 * \brief Set the rectangle to be displayed
 */
void	imagedisplaywidget::setImageRectangle(const ImageRectangle& imagerectangle) {
	displayRectangle(imagerectangle);
	if (imageRectangleEnabled()) {
		processNewSettings();
	}
}

/**
 * \brief Set the QRect to be displayed
 *
 * The QRect is using current image coordinates, which may depend on the
 * scaling applied.
 */
void	imagedisplaywidget::setImageRectangle(const QRect& rect) {
	// compute the rectangle 
	int	x, y, width, height;
	width = rect.size().width();
	height = rect.size().height();
	x = rect.topLeft().x();
	y = selectable->size().height() - rect.topLeft().y() - height - 1;

	// change scale
	int	s = image2pixmap.scale();
	if (s > 0) {
		x >>= s;
		y >>= s;
		height >>= s;
		width >>= s;
	}
	if (s < 0) {
		x <<= -s;
		y <<= -s;
		height <<= -s;
		width <<= -s;
	}

	// if we are currently displaying a subimage
	if (imageRectangleEnabled()) {
		x += _rectangle.origin().x();
		y += _rectangle.origin().y();
	}

	// create the rectangle
	ImageRectangle	r(ImagePoint(x, y), ImageSize(width, height));
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"QRect=%dx%d@(%d,%d) -> ImageRectangle(%s)",
		rect.size().width(), rect.size().height(),
		rect.topLeft().x(), rect.topLeft().y(),
		r.toString().c_str());
	setImageRectangle(r);
}

/**
 * \brief Show whether the rectangle is displayed or not
 */
bool	imagedisplaywidget::imageRectangleEnabled() {
	return ui->subframeBox->isChecked();
}

/**
 * \brief Set whether the rectangle is displayed
 */
void	imagedisplaywidget::setImageRectangleEnabled(bool y) {
	ui->subframeBox->setChecked(y);
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

/**
 * \brief Change the width
 *
 * This method changes the width of the subrectangle. If the new subrectangle
 * does not fit into the image, the x coordinate is modified too, so that
 * the new rectangle can still fit the image
 */
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

/**
 * \brief Change the height of the subrectangle
 *
 * This method changes th height of the subrectangle. If the new subrectangle
 * does not fit into the iamge, the y coordinate is modified too, so that
 * the new rectangle can still fit the image.
 */
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

/**
 * \brief Change the X coordinate of the lower left corner of the image
 *
 * This method changes the x coordinate of the lower left cornder of the
 * subrectangle. If the new subrectangle does not fit into the image, 
 * the width is changed so that it can still fit the image.
 */
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

/**
 * \brief Change the Y coordinate of the lower left corner of the image
 *
 * This method changes the y coordinate of the lower left corner of the
 * subrectangle. If the new subrectangle does not fit into the image, 
 * the height is changed so that it can still fit the image.
 */
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

/**
 * \brief Change the rectangle without any signals being fired
 *
 * When we change the rectangle, e.g. when the new rectangle comes from
 * an external source, we don't want any signals to be fired, because
 * that would mess up redisplay of the image.
 *
 * As a side effect, thie method also always sets the _rectangle member,
 * to make sure that _rectangle always reflects the current setting of
 * the rectangle controls. This also allows to use subrectangles even
 * if the subframe control area is not displayed.
 */
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

	_rectangle = r;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rectangle set");
}

/**
 * \brief set the new image
 *
 * This method just remembers the new image and emits the imageUpdated
 * signal. The main thread will then execute the processNewImage
 * method to actually display the image.
 */
void	imagedisplaywidget::setImage(astro::image::ImagePtr image) {
	_image = image;
	emit imageUpdated();
}

/**
 * \brief Processing for image info of a new image
 *
 * This method is also called when the image info is enabled, as the current
 * image info may not have any information in it, or information from a
 * previous image.
 */
void	imagedisplaywidget::processNewImageInfo(ImagePtr image) {
	if (!image) {
		return;
	}

	// there is no need to do anything if the info area is not visible
	if (!ui->infoFrame->isVisible()) {
		return;
	}

	// retrieve image information and update the info field
	if (image->hasMetadata("INSTRUME")) {
		std::string	instrument = image->getMetadata("INSTRUME");
		ui->instrumentField->setText(QString(instrument.c_str()));
	} else {
		ui->instrumentField->setText(QString("(unknown)"));
	}

	// image size and binning
	std::string	sizeinfo = image->getFrame().toString();
	int	xbin = 0, ybin = 0;
	if (image->hasMetadata("XBINNING")) {
		xbin = image->getMetadata("XBINNING");
	}
	if (image->hasMetadata("YBINNING")) {
		ybin = image->getMetadata("YBINNING");
	}
	if ((xbin > 0) && (ybin > 0)) {
		sizeinfo = sizeinfo + " / " + Binning(xbin, ybin).toString();
	}
	ui->geometryField->setText(sizeinfo.c_str());

	// pixel type
	std::string	pixeltype
		= astro::demangle(image->pixel_type().name());
	if (0 == pixeltype.compare(0, 14, "astro::image::")) {
		pixeltype = pixeltype.substr(14);
	}
	ui->pixeltypeField->setText(QString(pixeltype.c_str()));

	// read pixel value statistics
	double	maximum = 0;
	double	minimum = 0;
	double	mean = 0;
	if (3 == image->planes()) {
		maximum = astro::image::filter::max_luminance(image);
		minimum = astro::image::filter::min_luminance(image);
		mean = astro::image::filter::mean_luminance(image);
	} else {
		maximum = astro::image::filter::max(image);
		minimum = astro::image::filter::min(image);
		mean = astro::image::filter::mean(image);
	}
	std::string	minmax;
	if (maximum > 100) {
		minmax = astro::stringprintf("%.0f/%.0f/%.0f",
			minimum, mean, maximum);
	} else if (maximum > 1) {
		minmax = astro::stringprintf("%.2f/%.2f/%.2f",
			minimum, mean, maximum);
	} else {
		minmax = astro::stringprintf("%.3f/%.3f/%.3f",
			minimum, mean, maximum);
	}
	ui->minmaxField->setText(QString(minmax.c_str()));

	// query exposure time
	if (image->hasMetadata("EXPTIME")) {
		Metavalue	v = image->getMetadata("EXPTIME");
		ui->exposuretimeField->setText(QString(v.getValue().c_str()));
	} else {
		ui->exposuretimeField->setText(QString("unknown"));
	}

	// query the bayer pattern
	if (image->hasMetadata("BAYER")) {
		Metavalue	v = image->getMetadata("EXPTIME");
		ui->bayerField->setText(QString(v.getValue().c_str()));
	} else {
		ui->bayerField->setText(QString("none"));
	}

	// read meta data from the image and display in the FITS info area
	QTableWidget	*table = ui->fitsinfoTable;
	table->setRowCount(image->nMetadata());
	int	row = 0;
	for_each(image->begin(), image->end(),
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
}

/**
 * \brief Processing of a new image related to the rectangle
 */
void	imagedisplaywidget::processNewImageRectangle(ImagePtr image) {
	if (!image) {
		return;
	}

	// ensure the maximum values the subframe controls can move 
	// stays within the bounds of the image
	ui->subframewidthBox->setMaximum(image->size().width());
	ui->subframeheightBox->setMaximum(image->size().height());
	ui->subframexBox->setMaximum(image->size().width() - 1);
	ui->subframeyBox->setMaximum(image->size().height() - 1);

	// check whether the current rectangle fits inside the new image
	if (_rectangle.isEmpty()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"empty rectangle, use image size");
		displayRectangle(image->size());
	} else {
		if (!image->size().bounds(_rectangle)) {
			// XXX find a rectangle that works
			displayRectangle(image->size());
		}
	}

	// the subframe group was so far disabled, but now that we have an
	// image, we enable it.
	ui->subframeGroup->setEnabled(true);
}

/**
 * \brief actually display the image after the settings have changed
 */
void	imagedisplaywidget::processDisplayImage(ImagePtr image) {
	if (!image) {
		return;
	}

	// get information about the size, we'll need that throughout
	ImageSize	size = image->size();

	// if the subframe box button is checked, then we have to make
	// sure the image2pixmap converter uses the current _rectangle 
	// setting, otherwise it should use the full frame of the image
	if (imageRectangleEnabled()) {
		image2pixmap.rectangle(_rectangle);
	} else {
		image2pixmap.rectangle(ImageRectangle());
	}

	// remember the current position of the scroll area
	int	hpos = ui->imageArea->horizontalScrollBar()->value();
	int	vpos = ui->imageArea->verticalScrollBar()->value();
	QSize	previoussize = ui->imageArea->widget()->size();
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"hpos = %d, vpos = %d, previous size=%d,%d",
		hpos, vpos, previoussize.width(), previoussize.height());

	// create a new selectable image and pixmap
	selectable = new SelectableImage();
	selectable->setRectangleSelectionEnabled(true);


	QPixmap *pixmap = image2pixmap(image);
	if (NULL != pixmap) {
		selectable->setPixmap(*pixmap);
	}
	selectable->setFixedSize(pixmap->width(), pixmap->height());
	selectable->setMinimumSize(pixmap->width(), pixmap->height());
	connect(selectable, SIGNAL(rectangleSelected(QRect*)),
		this, SLOT(rectangleSelected(QRect*)));

	// display the image
	ui->imageArea->setWidget(selectable);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new position: %d/%d", hpos, vpos);
	QSize	newsize = pixmap->size();
	hpos = newsize.width() * hpos / previoussize.width();
	vpos = newsize.height() * vpos / previoussize.height();
	ui->imageArea->horizontalScrollBar()->setValue(hpos);
	ui->imageArea->verticalScrollBar()->setValue(vpos);
	ui->imageArea->show();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image display complete");

	// XXX possible memory leak
	// delete pixmap;

	// update the histogram, if info is enabled
	if (infoIsVisible()) {
		QPixmap *histogram = image2pixmap.histogram(
			ui->histogramLabel->width(),
			ui->histogramLabel->height());
		if (NULL != histogram) {
			ui->histogramLabel->setPixmap(*histogram);
		}
		// XXX possible memory leak
		// delete histogram;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "histogram display complete");
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no histogram display");
	}
}

/**
 * \brief Processing done for a new image 
 */
void	imagedisplaywidget::processNewImage() {
	if (!_image) {
		return;
	}

	// make sure we always use the same image during the processing,
	// even if in the mean time a new image has arrived.
	ImagePtr	image = _image;

	// process rectangle information for the new image
	processNewImageRectangle(image);

	// process general image info for the new image
	processNewImageInfo(image);

	// do the processing that depends on the settings
	processDisplayImage(image);
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

	// display the image
	processDisplayImage(_image);
}

/**
 * \brief Display the gain settings obtained from an autogain computation
 */
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
	// check whether the gain slider has changed
	if (sender() == ui->gainSlider) {
		image2pixmap.gain(displayGainSetting());
	}
	// check whether the brightness slider has changed
	if (sender() == ui->brightnessSlider) {
		image2pixmap.brightness(displayBrightnessSetting());
	}
	// check whether the autogain button was clicked. This initiates
	// computation of suitable gain settings.
	if (sender() == ui->autogainButton) {
		if (imageRectangleEnabled()) {
			AutoGain	autogain(_image, _rectangle);
			displayAutoGain(autogain);
		} else {
			AutoGain	autogain(_image);
			displayAutoGain(autogain);
		}
	}
	// check whether the scale setting has changed
	if (sender() == ui->scaleDial) {
		image2pixmap.scale(displayScaleSetting());
	}
	// check whether the logarithm box was toggled
	if (sender() == ui->logarithmicBox) {
		image2pixmap.logarithmic(ui->logarithmicBox->isChecked());
	}
	// check whether the width of the subframe was changed
	if (sender() == ui->subframewidthBox) {
		displayRectangle(displayWidthSetting());
		if (!imageRectangleEnabled()) {
			return;
		}
	}
	// check whether the height of the subframe was changed
	if (sender() == ui->subframeheightBox) {
		displayRectangle(displayHeightSetting());
		if (!imageRectangleEnabled()) {
			return;
		}
	}
	// check whether the x coordinate of the subframe was changed
	if (sender() == ui->subframexBox) {
		displayRectangle(displayXSetting());
		if (!imageRectangleEnabled()) {
			return;
		}
	}
	// check whether the y coordinate of the subframe was changed
	if (sender() == ui->subframeyBox) {
		displayRectangle(displayYSetting());
		if (!imageRectangleEnabled()) {
			return;
		}
	}
	// check whether the subframe yes/no button was clicked
	if (sender() == ui->subframeBox) {
		// nothing needs to be done, as we always check the
		// button directly
	}
	if (sender() == ui->subframefullButton) {
		displayRectangle(ImageRectangle(_image->size()));
		if (!imageRectangleEnabled()) {
			return;
		}
	}
	// with these new settings we should now display the image
	processNewSettings();
}

/**
 * \brief convert a selected rectangle 
 */
void	imagedisplaywidget::rectangleSelected(QRect* rect) {
	setImageRectangle(*rect);
}

} // namespace snowgui
