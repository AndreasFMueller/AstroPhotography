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
#include <QTimer>

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

	void	redisplay();

private:
	Ui::instrumentswindow *ui;
	astro::discover::ServiceObject	_serviceobject;
	snowstar::InstrumentsPrx	_instruments;
	snowstar::InstrumentPrx	_instrument;
	snowstar::ModulesPrx	_modules;
	astro::discover::ServiceDiscoveryPtr	_discovery;
	QTimer	*_discoveryTimer;

	void	instrumentEnabled(bool);

public slots:
	void	instrumentSelected(QString);
	void	serviceSelected(QString);
	void	checkdiscovery();

	void	addClicked();
	void	addguiderccdClicked();
	void	deleteClicked();

	void	deleteInstrument();

protected:
	virtual void	closeEvent(QCloseEvent *event);
};


} // namespace snowgui

#endif // SNOWGUI_INSTRUMENTSWINDOW_H
