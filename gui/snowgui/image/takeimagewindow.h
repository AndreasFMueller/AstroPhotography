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

public:
	explicit takeimagewindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~takeimagewindow();

private:
	Ui::takeimagewindow *ui;

public slots:
	void	receiveImage(astro::image::ImagePtr image);
	void	rectangleSelected(astro::image::ImageRectangle);

protected:
	void	closeEvent(QCloseEvent *);
};

} // namespace snowgui

#endif // SNOWGUI_TAKEIMAGEWINDOW_H
