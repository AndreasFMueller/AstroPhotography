/*
 * imagedisplaywidget.cpp -- FITS image display implementation
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "imagedisplaywidget.h"
#include "ui_imagedisplaywidget.h"
#include <AstroDebug.h>

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
 * \brief 
 */
void	imagedisplaywidget::processNewImage() {
	// retrieve image information and update the info field

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
void	imagedisplaywidgeht::imageSettingsChanged() {
	// read
	processNewSettings();
}
