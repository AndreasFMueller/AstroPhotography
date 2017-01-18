/*
 * imagedetailwidget.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_IMAGEDETAILWIDGET_H
#define SNOWGUI_IMAGEDETAILWIDGET_H

#include <QWidget>
#include <image.h>
#include <AstroImage.h>

namespace snowgui {

namespace Ui {
	class imagedetailwidget;
}

class imagedetailwidget : public QWidget {
	Q_OBJECT

public:
	explicit imagedetailwidget(QWidget *parent = 0);
	~imagedetailwidget();

	void	setImage(snowstar::ImagePrx image);

signals:
	void	imageReceived(astro::image::ImagePtr);
	void	deleteCurrentImage();

public slots:
	void	loadImage();
	void	deleteImage();
	void	saveImage();

private:
	Ui::imagedetailwidget *ui;
	snowstar::ImagePrx	_image;
	astro::image::ImagePtr	_imageptr;
};

} // namespace snowgui
#endif // SNOWGUI_IMAGEDETAILWIDGET_H
