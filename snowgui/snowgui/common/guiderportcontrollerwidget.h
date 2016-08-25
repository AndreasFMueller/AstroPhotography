/*
 * guiderportcontrollerwidget.h -- guider controller widget declaration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDERPORTCONTROLLERWIDGET_H
#define GUIDERPORTCONTROLLERWIDGET_H

#include <InstrumentWidget.h>

namespace snowgui {

namespace Ui {
	class guiderportcontrollerwidget;
}

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
	void	guiderportSelected(int);

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

#endif // GUIDERPORTCONTROLLERWIDGET_H
