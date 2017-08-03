/*
 * takeimagewindow.h -- widget for taking images
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TAKEIMAGEWINDOW_H
#define SNOWGUI_TAKEIMAGEWINDOW_H

#include <InstrumentWidget.h>

namespace snowgui {

namespace Ui {
	class takeimagewindow;
}

class takeimagewindow : public snowgui::InstrumentWidget {
	Q_OBJECT

	astro::image::ImagePtr	_image;

public:
	explicit takeimagewindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~takeimagewindow();

private:
	Ui::takeimagewindow *ui;

signals:
	void	offerImage(astro::image::ImagePtr);

public slots:
	void	receiveImage(astro::image::ImagePtr image);
	void	rectangleSelected(astro::image::ImageRectangle);

protected:
	void	closeEvent(QCloseEvent *);
	void	changeEvent(QEvent *);
};


} // namespace snowgui
#endif // SNOWGUI_TAKEIMAGEWINDOW_H
