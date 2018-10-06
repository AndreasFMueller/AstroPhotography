/*
 * focusingwindow.h -- Widget for focusing
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef FOCUSINGWIDGET_H
#define FOCUSINGWIDGET_H

#include <InstrumentWidget.h>
#include <AstroImage.h>

namespace snowgui {

namespace Ui {
	class focusingwindow;
}

class focusingwindow : public snowgui::InstrumentWidget {
	Q_OBJECT

public:
	explicit focusingwindow(QWidget *parent);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	~focusingwindow();

private:
	Ui::focusingwindow *ui;

public slots:
	void	receiveImage(astro::image::ImagePtr image);
	void	rectangleSelected(astro::image::ImageRectangle);
protected:
	void	closeEvent(QCloseEvent *);
};

} // namespace snowgui

#endif // FOCUSINGWIDGET_H
