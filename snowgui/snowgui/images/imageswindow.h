/*
 * imageswindow.h -- preview window for images stored on the server
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_IMAGESWINDOW_H
#define SNOWGUI_IMAGESWINDOW_H

#include <QWidget>
#include <image.h>
#include <AstroDiscovery.h>
#include <QTreeWidgetItem>
#include <AstroImage.h>

namespace snowgui {

namespace Ui {
	class imageswindow;
}

class imageswindow : public QWidget {
	Q_OBJECT

public:
	imageswindow(QWidget *parent, astro::discover::ServiceObject serviceobject);
	~imageswindow();

	void	setImages(snowstar::ImagesPrx images);

public slots:
	void	currentImageChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void	setImage(astro::image::ImagePtr);
	void	deleteCurrentImage();
	void	itemDoubleClicked(QTreeWidgetItem *, int);

private:
	Ui::imageswindow *ui;
	astro::discover::ServiceObject	_serviceobject;
	snowstar::ImagesPrx	_images;

protected:
	void	closeEvent(QCloseEvent *);
};


} // namespace snowgui
#endif // SNOWGUI_IMAGESWINDOW_H
