/*
 * filterwheelcontrollerwidget.h -- controller for filterwheel
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef FILTERWHEELCONTROLLERWIDGET_H
#define FILTERWHEELCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <QTimer>

namespace snowgui {

namespace Ui {
	class filterwheelcontrollerwidget;
}

class filterwheelcontrollerwidget;

/**
 * \brief update thread
 *
 * The idea of the update thread is that it does all the possibly lengthy
 * stuff that happends when updateing information from the server in a
 * separate thread. It does this by calling the statusUpdate method of the
 * main class. The main class then emits signals that it understands. This
 * queues the new data on the event loop of the main thread for the GUI
 * to quickly integrate.
 *
 * It should be easy to turn this thread class into a template class for
 * other device controller widgets to use in a similar way.
 */
class filterwheelupdatethread : public QThread {
	Q_OBJECT
	filterwheelcontrollerwidget	*_filterwheelcontrollerwidget;
public:
	filterwheelupdatethread(filterwheelcontrollerwidget *fwc)
		: QThread(NULL), _filterwheelcontrollerwidget(fwc) {
	}
	~filterwheelupdatethread() {
	}
public slots:
	void	statusUpdate();
	void	positionUpdate();
};

/**
 * \brief A reusable component to control a filter wheel
 */
class filterwheelcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::FilterWheelPrx	_filterwheel;
	snowstar::FilterwheelState	_previousstate;
	int				_position;
	filterwheelupdatethread		*_updatethread;
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
	QTimer	statusTimer;

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
