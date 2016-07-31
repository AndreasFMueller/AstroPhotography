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

namespace Ui {
	class imagedisplaywidget;
}

class imagedisplaywidget : public QWidget
{
	Q_OBJECT

	astro::image::ImagePtr	_image;
	snowgui::Image2Pixmap	image2pixmap;

public:
	explicit imagedisplaywidget(QWidget *parent = 0);
	~imagedisplaywidget();

	void	setImage(astro::image::ImagePtr image);

private:
	Ui::imagedisplaywidget *ui;

	double	displayGainSetting();
	double	displayBrightnessSetting();
	double	displayScaleSetting();
signals:
	void	imageUpdated();

public slots:
	void	processNewImage();
	void	processNewSettings();
	void	imageSettingsChanged();
};

#endif // IMAGEDISPLAYWIDGET_H
