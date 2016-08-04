/*
 * guiderportcontrollerwidget.h -- guider controller widget declaration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDERCONTROLLERWIDGET_H
#define GUIDERCONTROLLERWIDGET_H

#include <InstrumentWidget.h>

namespace Ui {
	class guiderportcontrollerwidget;
}

namespace snowgui {

class guiderportcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::GuiderPortPrx	_guiderport;
	float	_activationtime;
public:
	explicit guiderportcontrollerwidget(QWidget *parent = 0);
	~guiderportcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);

signals:
	void	activationTimeChanged();

private:
	Ui::guiderportcontrollerwidget *ui;

	void	setupGuiderport();

public slots:
	void	guiderportChanged(int);
	void	activateRAplus();
	void	activateRAminus();
	void	activateDECplus();
	void	activateDECminus();
	void	setActivationTime(double);
	void	changeActivationTime(double);
};

} // namespace snowogui

#endif // GUIDERCONTROLLERWIDGET_H
