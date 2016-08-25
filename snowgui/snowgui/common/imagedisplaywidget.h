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
class imagedisplaywidget : public QWidget
{
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
	void	setGainVisible(bool);
	bool	scaleIsVisible();
	void	setScaleVisible(bool);
	bool	subframeIsVisible();
	void	setSubframeVisible(bool);

	// control whether the information is visible
	bool	infoIsVisible();
	void	setInfoVisible(bool);

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
	void	pointSelected(astro::image::ImagePoint);

public slots:
	void	processNewImage();
	void	processNewSettings();
	void	imageSettingsChanged();
	void	selectRectangle(QRect);
	void	selectPoint(QPoint);
	void	receiveImage(astro::image::ImagePtr);
};

} // namespace snowgui

#endif // IMAGEDISPLAYWIDGET_H
