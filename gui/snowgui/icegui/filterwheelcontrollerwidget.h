/*
 * filterwheelcontrollerwidget.h -- controller for filterwheel
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef FILTERWHEELCONTROLLERWIDGET_H
#define FILTERWHEELCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <QTimer>
#include <camera.h>
#include "CallbackIdentity.h"

namespace snowgui {

namespace Ui {
	class filterwheelcontrollerwidget;
}

class filterwheelcontrollerwidget;

class FilterWheelCallbackI : public QObject,
	public snowstar::FilterWheelCallback, public CallbackIdentity {
	Q_OBJECT

public:
	FilterWheelCallbackI();
	void	state(const snowstar::FilterwheelState state,
			const Ice::Current& current);
	void	position(const int position,
			const Ice::Current& current);
	void	stop(const Ice::Current& current);
signals:
	void	callbackState(snowstar::FilterwheelState);
	void	callbackPosition(int);
};

/**
 * \brief A reusable component to control a filter wheel
 */
class filterwheelcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::FilterWheelPrx	_filterwheel;
	snowstar::FilterwheelState	_previousstate;
	int				_position;

	Ice::ObjectPtr	_filterwheel_callback;
	Ice::Identity	identity();
public:
	explicit filterwheelcontrollerwidget(QWidget *parent = 0);
	~filterwheelcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	void	statusUpdate();
	void	positionUpdate();

signals:
	void	filterwheelSelected(snowstar::FilterWheelPrx);
	void	filterwheelSelected(int);

	// signals emitted when the filterwheel state changes
	void	filterwheelStart();
	void	filterwheelStop();
	void	filterwheelStateChanged(snowstar::FilterwheelState);
	void	filterwheelPositionChanged(int filterindex);

private:
	Ui::filterwheelcontrollerwidget *ui;

	void	setupFilterwheel();
	void	displayFilter(int index);

public slots:
	void	setFilter(int index);
	void	filterwheelChanged(int);
	void	filterwheelNewState(snowstar::FilterwheelState);
	void	filterwheelNewPosition(int);

	void	callbackState(snowstar::FilterwheelState);
	void	callbackPosition(int);
	friend class FilterWheelCallbackI;
};

} // namespace snowgui

#endif // FILTERWHEELCONTROLLERWIDGET_H
