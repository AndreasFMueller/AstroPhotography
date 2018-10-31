/*
 * focusingcontrollerwidget.h -- widget to control the focusing process
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_FOCUSINGCONTROLLERWIDGET_H
#define SNOWGUI_FOCUSINGCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <focusing.h>
#include <AstroImage.h>
#include <QTimer>

#include <QWidget>

namespace snowgui {

namespace Ui {
	class focusingcontrollerwidget;
}

class focusingcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::FocusingFactoryPrx	_focusingfactory;
	snowstar::FocusingPrx		_focusing;

	snowstar::CcdPrx		_ccd;
	snowstar::FocuserPrx		_focuser;

	std::string	_ccdname;
	std::string	_focusername;

	int	_center;
	int	_stepsize;
	int	_steps;

	astro::camera::Exposure		_exposure;

	snowstar::FocusState	_previousstate;

	QTimer	_timer;

public:
	explicit focusingcontrollerwidget(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject service,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	~focusingcontrollerwidget();

private:
	Ui::focusingcontrollerwidget *ui;

	void	start();
	void	stop();

public slots:
	void	statusUpdate();
	void	startClicked();
	void	stepsChanged(int);
	void	stepsizeChanged(int);
	void	centerChanged(int);
	void	exposureChanged(astro::camera::Exposure);
};


} // namespace snowgui
#endif // SNOWGUI_FOCUSINGCONTROLLERWIDGET_H
