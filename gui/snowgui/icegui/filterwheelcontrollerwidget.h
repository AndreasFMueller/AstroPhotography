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
	Ice::Identity	_identity;
public:
	FilterWheelCallbackI(filterwheelcontrollerwidget& f);
	~FilterWheelCallbackI();
	const Ice::Identity	identity() const { return _identity; }
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

	FilterWheelCallbackI	*_filterwheel_cb;
	Ice::ObjectPtr	_filterwheel_ptr;
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

	// signals emiited by the callbacks
	void	callbackStateChanged(snowstar::FilterwheelState);
	void	callbackPositionChanged(int);

private:
	Ui::filterwheelcontrollerwidget *ui;

	void	setupFilterwheel();
	void	displayFilter(int index);

public slots:
	void	setFilter(int index);
	void	filterwheelChanged(int);
	void	filterwheelNewState(snowstar::FilterwheelState);
	void	filterwheelNewPosition(int);

private:
	void	callbackState(snowstar::FilterwheelState);
	void	callbackPosition(int);
	friend class FilterWheelCallbackI;
};

} // namespace snowgui

#endif // FILTERWHEELCONTROLLERWIDGET_H
