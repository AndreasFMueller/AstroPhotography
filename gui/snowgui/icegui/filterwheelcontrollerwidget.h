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

namespace snowgui {

namespace Ui {
	class filterwheelcontrollerwidget;
}

class filterwheelcontrollerwidget;

class FilterWheelCallbackI : public snowstar::FilterWheelCallback {
	filterwheelcontrollerwidget&	_filterwheelcontrollerwidget;
public:
	FilterWheelCallbackI(filterwheelcontrollerwidget& f);
	~FilterWheelCallbackI();
	void	state(const snowstar::FilterwheelState state,
			const Ice::Current& current);
	void	position(const int position,
			const Ice::Current& current);
	void	stop(const Ice::Current& current);
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
	Ice::Identity	_filterwheel_identity;
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
};

} // namespace snowgui

#endif // FILTERWHEELCONTROLLERWIDGET_H
