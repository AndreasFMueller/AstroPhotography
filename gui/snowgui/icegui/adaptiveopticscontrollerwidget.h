/*
 * adaptiveopticscontrollerwidget.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_ADAPTIVEOPTICSCONTROLLERWIDGET_H
#define SNOWGUI_ADAPTIVEOPTICSCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <QTimer>

namespace snowgui {

namespace Ui {
	class adaptiveopticscontrollerwidget;
}

class adaptiveopticscontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::AdaptiveOpticsPrx	_adaptiveoptics;

public:
	explicit adaptiveopticscontrollerwidget(QWidget *parent = 0);
	~adaptiveopticscontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();

signals:
	void	adaptiveopticsSelected();

private:
	Ui::adaptiveopticscontrollerwidget *ui;

	void	setupAdaptiveOptics();

	QTimer  statusTimer;

public slots:
	void    adaptiveopticsChanged(int);
	void	setPoint(QPointF);
	void	statusUpdate();
};

} // namespace snowgui
#endif // SNOWGUI_ADAPTIVEOPTICSCONTROLLERWIDGET_H
