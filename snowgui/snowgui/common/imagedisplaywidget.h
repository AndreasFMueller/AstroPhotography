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

namespace Ui {
	class imagedisplaywidget;
}

namespace snowgui {

/**
 * \brief common image display widget
 *
 * This widget includes controls to change brightness and contrast, and
 * displays a statistics and metadata information about the image
 */
class imagedisplaywidget : public QWidget
{
	Q_OBJECT

	astro::image::ImagePtr		_image;
	astro::image::ImageRectangle	_rectangle;
	snowgui::Image2Pixmap		image2pixmap;

public:
	explicit imagedisplaywidget(QWidget *parent = 0);
	~imagedisplaywidget();

	void	setImage(astro::image::ImagePtr image);

	bool	settingsIsVisible();
	void	setSettingsVisible(bool);
	bool	gainIsVisible();
	void	setGainVisible(bool);
	bool	scaleIsVisible();
	void	setScaleVisible(bool);
	bool	subframeIsVisible();
	void	setSubframeVisible(bool);
	bool	infoIsVisible();
	void	setInfoVisible(bool);

private:
	Ui::imagedisplaywidget *ui;

	double	displayGainSetting();
	double	displayBrightnessSetting();
	int	displayScaleSetting();
	ImageRectangle	displayWidthSetting();
	ImageRectangle	displayHeightSetting();
	ImageRectangle	displayXSetting();
	ImageRectangle	displayYSetting();
	void	displayAutoGain(const AutoGain& autogain);
	void	displayRectangle(const astro::image::ImageRectangle& rectangle);
signals:
	void	imageUpdated();

public slots:
	void	processNewImage();
	void	processNewSettings();
	void	imageSettingsChanged();
};

} // namespace snowgui

#endif // IMAGEDISPLAYWIDGET_H
