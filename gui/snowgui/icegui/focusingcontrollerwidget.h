/*
 * focusingcontrollerwidget.h -- widget to control the focusing process
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_FOCUSINGCONTROLLERWIDGET_H
#define SNOWGUI_FOCUSINGCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <focusing.h>
#include <repository.h>
#include <AstroImage.h>
#include <QTimer>
#include <QWidget>
#include <FocusingCallbackI.h>
#include <CommonClientTasks.h>

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

	snowstar::RepositoriesPrx	_repositories;

	std::string	_ccdname;
	std::string	_focusername;

	int	_center;
	int	_stepsize;
	int	_steps;
	std::string	_repository;

	astro::camera::Exposure		_exposure;

	snowstar::FocusState	_previousstate;

	QTimer	_timer;

	FocusingCallbackI	*_callback;
	Ice::ObjectPtr	callback;
	Ice::Identity   _ident;
	snowstar::CallbackAdapterPtr    _adapter;

public:
	explicit focusingcontrollerwidget(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject service,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	~focusingcontrollerwidget();

signals:
	void    pointReceived(snowstar::FocusPoint);
        void    stateReceived(snowstar::FocusState);
        void    focuselementReceived(snowstar::FocusElement);

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
	void	repositoryChanged(const QString& text);

	void	receivePoint(snowstar::FocusPoint);
	void	receiveState(snowstar::FocusState);
	void	receiveFocusElement(snowstar::FocusElement);
};


} // namespace snowgui
#endif // SNOWGUI_FOCUSINGCONTROLLERWIDGET_H
