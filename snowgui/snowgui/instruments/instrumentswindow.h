/*
 * instrumentswindow.h -- top level window for instruments application
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_INSTRUMENTSWINDOW_H
#define SNOWGUI_INSTRUMENTSWINDOW_H

#include <QWidget>
#include <AstroDiscovery.h>
#include <instruments.h>
#include <RemoteInstrument.h>

namespace snowgui {

namespace Ui {
	class instrumentswindow;
}

class instrumentswindow : public QWidget {
	Q_OBJECT

public:
	instrumentswindow(QWidget *parent,
		astro::discover::ServiceObject serviceobject);
	~instrumentswindow();

private:
	Ui::instrumentswindow *ui;
	astro::discover::ServiceObject	_serviceobject;
	snowstar::InstrumentsPrx	_instruments;
	snowstar::InstrumentPrx	_instrument;

public slots:
	void	instrumentSelected(QString);
};


} // namespace snowgui

#endif // SNOWGUI_INSTRUMENTSWINDOW_H
