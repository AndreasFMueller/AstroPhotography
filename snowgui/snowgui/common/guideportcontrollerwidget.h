/*
 * guideportcontrollerwidget.h -- guider controller widget declaration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDEPORTCONTROLLERWIDGET_H
#define GUIDEPORTCONTROLLERWIDGET_H

#include <InstrumentWidget.h>

namespace snowgui {

namespace Ui {
	class guideportcontrollerwidget;
}

class guideportcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::GuidePortPrx	_guideport;
	float	_activationtime;
public:
	explicit guideportcontrollerwidget(QWidget *parent = 0);
	~guideportcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);

signals:
	void	activationTimeChanged();
	void	guideportSelected(int);

private:
	Ui::guideportcontrollerwidget *ui;

	void	setupGuideport();

public slots:
	void	guideportChanged(int);
	void	activateRAplus();
	void	activateRAminus();
	void	activateDECplus();
	void	activateDECminus();
	void	setActivationTime(double);
	void	changeActivationTime(double);
};

} // namespace snowogui

#endif // GUIDEPORTCONTROLLERWIDGET_H
