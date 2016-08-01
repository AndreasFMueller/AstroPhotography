/*
 * filterwheelcontrollerwidget.h -- controller for filterwheel
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef FILTERWHEELCONTROLLERWIDGET_H
#define FILTERWHEELCONTROLLERWIDGET_H

#include <InstrumentWidget.h>

namespace Ui {
	class filterwheelcontrollerwidget;
}

namespace snowgui {

class filterwheelcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::FilterWheelPrx	_filterwheel;
public:
	explicit filterwheelcontrollerwidget(QWidget *parent = 0);
	~filterwheelcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);

signals:
	void	filterInstalled();

private:
	Ui::filterwheelcontrollerwidget *ui;
	QTimer	*statusTimer;

	void	setupFilterwheel();
	void	displayFilter(int index);


public slots:
	void	setFilter(int index);
	void	guiChanged();
	void	filterwheelChanged(int);
	void	statusUpdate();
};

} // namespace snowgui

#endif // FILTERWHEELCONTROLLERWIDGET_H
