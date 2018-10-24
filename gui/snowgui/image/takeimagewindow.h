/*
 * takeimagewindow.h -- widget for taking images
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TAKEIMAGEWINDOW_H
#define SNOWGUI_TAKEIMAGEWINDOW_H

#include <InstrumentWidget.h>
#include <TakeImageSink.h>

namespace snowgui {

namespace Ui {
	class takeimagewindow;
}

class takeimagewindow : public snowgui::InstrumentWidget {
	Q_OBJECT

	snowstar::CcdPrx	_ccd;

public:
	explicit takeimagewindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	~takeimagewindow();

	TakeImageSink	*_imagesink;
	Ice::Identity	_sinkidentity;

private:
	Ui::takeimagewindow *ui;

signals:
	void	startStream();

public slots:
	void	receiveImage(astro::image::ImagePtr image);
	void	rectangleSelected(astro::image::ImageRectangle);
	void	setupStream();
	void	setCcd(snowstar::CcdPrx);
	void	streamFinished();

protected:
	void	closeEvent(QCloseEvent *);
};

} // namespace snowgui

#endif // SNOWGUI_TAKEIMAGEWINDOW_H
