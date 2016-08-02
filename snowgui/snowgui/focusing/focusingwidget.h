/*
 * focusingwidget.h -- Widget for focusing
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef FOCUSINGWIDGET_H
#define FOCUSINGWIDGET_H

#include <InstrumentWidget.h>

namespace Ui {
	class focusingwidget;
}

class focusingwidget : public snowgui::InstrumentWidget {
	Q_OBJECT

public:
	explicit focusingwidget(QWidget *parent);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~focusingwidget();

private:
	Ui::focusingwidget *ui;

public slots:
	void	imageReceived();
	void	rectangleSelected(astro::image::ImageRectangle);
};

#endif // FOCUSINGWIDGET_H
