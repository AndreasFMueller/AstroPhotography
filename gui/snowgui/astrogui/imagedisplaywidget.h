/*
 * imagedisplayidget.h -- display a FITS image
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef IMAGEDISPLAYWIDGET_H
#define IMAGEDISPLAYWIDGET_H

#include <QWidget>
#include <AstroImage.h>
#include <Image2Pixmap.h>
#include <AutoGain.h>
#include <QRubberBand>
#include <SelectableImage.h>

namespace snowgui {

namespace Ui {
	class imagedisplaywidget;
}

/**
 * \brief common image display widget
 *
 * This widget includes controls to change brightness and contrast, and
 * displays a statistics and metadata information about the image
 */
class imagedisplaywidget : public QWidget {
	Q_OBJECT

	// the current image, may be null if no image has been set yet
	astro::image::ImagePtr		_image;
	// the subframe select. This always reflects the current setting
	// of the subframe controls.
	astro::image::ImageRectangle	_rectangle;
	// the converter to convert images into pixmaps
	snowgui::Image2Pixmap		image2pixmap;
	// the enhanced QLabel that can perform a rubberband selection
	// of a subrectangle. This is mainly needed beause we have to
	// later be able to compute the coordinates based on the actual
	// image displayed
	snowgui::SelectableImage	*selectable;
	// whether or not to debayer, and what bayer pattern to use
	astro::image::MosaicType	_bayer_mosaic;
	// whether or not to show crosshairs
	bool	_crosshairs;
public:
	explicit imagedisplaywidget(QWidget *parent = 0);
	~imagedisplaywidget();

	// give the widget an image to display
	void	setImage(astro::image::ImagePtr image);

	// control whether the settings are visibale
	bool	settingsIsVisible();
	void	setSettingsVisible(bool);

	// the gain/scale/subframe settings can individually be made visible
	bool	gainIsVisible();
	bool	scaleIsVisible();
	bool	subframeIsVisible();
	bool	infoIsVisible();

	bool	crosshairs();
	void	crosshairs(bool);

	bool	horizontalFlip() const;
	bool	verticalFlip() const;

	bool	negative() const;

	bool	showRed() const;
	bool	showGreen() const;
	bool	showBlue() const;

	// control whether the image information at the bottom is visible
	// The next methods are for controlling the subrectangle display
	// A subrectangle can be selected even if the subframe controls
	// are not displayed
	astro::image::ImageRectangle	imageRectangle();
	void	setImageRectangle(const astro::image::ImageRectangle&);
	void	setImageRectangle(const QRect&);

	bool	imageRectangleEnabled();
	void	setImageRectangleEnabled(bool);

	// whether or not to accept point or rectangle selections
	bool	rectangleSelectionEnabled();
	void	setRectangleSelectionEnabled(bool);
	bool	pointSelectionEnabled();
	void	setPointSelectionEnabled(bool);

	// whether or not to debayer
	void	bayer_mosaic(astro::image::MosaicType m);
	astro::image::MosaicType	bayer_mosaic() const;

private:
	Ui::imagedisplaywidget *ui;

	// the display... methods ensure consistency of GUI elements with
	// internal state. So if a change in a GUI control requires other
	// GUI controls to be changed as well, the display method does
	// it, and it returns the new setting. The return value can then
	// be used to also modify the image display, if needd.
	double	displayGainSetting();
	double	displayBrightnessSetting();
	int	displayScaleSetting();
	ImageRectangle	displayWidthSetting();
	ImageRectangle	displayHeightSetting();
	ImageRectangle	displayXSetting();
	ImageRectangle	displayYSetting();
	void	displayAutoGain(const AutoGain& autogain);
	void	displayRectangle(const astro::image::ImageRectangle& rectangle);

	// conversion functions for the image coordinate system
	astro::image::ImagePoint	convertPoint(int x, int y);

	// some methods to better structure the processing when a new image
	// comes in
	void	processNewImageInfo(astro::image::ImagePtr image);
	void	processNewImageRectangle(astro::image::ImagePtr image);
	void	processDisplayImage(astro::image::ImagePtr image);
signals:
	void	imageUpdated();
	void	rectangleSelected(astro::image::ImageRectangle);
	void	rectangleSelected(QRect);
	void	pointSelected(astro::image::ImagePoint);
	void	offerImage(astro::image::ImagePtr, std::string);

public slots:
	void	processNewImage();
	void	processNewSettings();
	void	imageSettingsChanged();
	void	selectRectangle(QRect);
	void	selectPoint(QPoint);
	void	receiveImage(astro::image::ImagePtr);
	void	bayerChanged(int);
	void	crosshairsChanged(int);
	void	crosshairsCenter(astro::image::ImagePoint);

	void	redScaleChanged(double);
	void	greenScaleChanged(double);
	void	blueScaleChanged(double);
	void	redOffsetChanged(double);
	void	greenOffsetChanged(double);
	void	blueOffsetChanged(double);

	void	setGainVisible(bool);
	void	toggleGainVisible();

	void	setScaleVisible(bool);
	void	toggleScaleVisible();

	void	setSubframeVisible(bool);
	void	toggleSubframeVisible();

	void	setInfoVisible(bool);
	void	toggleInfoVisible();

	void	setCrosshairsVisible(bool);
	void	toggleCrosshairsVisible();

	void	setVerticalFlip(bool);
	void	toggleVerticalFlip();

	void	setHorizontalFlip(bool);
	void	toggleHorizontalFlip();

	void	setNegative(bool);
	void	toggleNegative();

	void	setShowRed(bool);
	void	setShowGreen(bool);
	void	setShowBlue(bool);
	void	toggleShowRed();
	void	toggleShowGreen();
	void	toggleShowBlue();

	void	showContextMenu(const QPoint& point);
private:
	void	closeEvent(QCloseEvent *);
	void	changeEvent(QEvent *);
};

} // namespace snowgui

#endif // IMAGEDISPLAYWIDGET_H
