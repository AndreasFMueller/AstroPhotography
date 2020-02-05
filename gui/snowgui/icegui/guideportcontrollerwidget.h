/*
 * guideportcontrollerwidget.h -- guider controller widget declaration
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDEPORTCONTROLLERWIDGET_H
#define GUIDEPORTCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <QTimer>
#include <IceConversions.h>
#include <camera.h>

namespace snowgui {

namespace Ui {
	class guideportcontrollerwidget;
}

class guideportcontrollerwidget;

/**
 * \brief Callback class for the guideport
 *
 * The callback needs to be a separate class because ICE has it's own
 * reference counting resource managment.
 *
 * Since we want to send messages from the callback object, it must inherit
 * from QObject. Apparently, the MOC compiler cannot handle this unless
 * QObject is the first in the list of superclasses. With QObject in second
 * position, an error is generated when compiling
 * moc_guideportcontrollerwidget.cpp
 */
class GuidePortCallbackI : public QObject, public snowstar::GuidePortCallback {
	Q_OBJECT

	guideportcontrollerwidget&	_guideportcontrollerwidget;
public:
	GuidePortCallbackI(guideportcontrollerwidget& g);
	void	activate(const snowstar::GuidePortActivation& activation,
			const Ice::Current& /* current */);
signals:
	void	activation(astro::camera::GuidePortActivation);
};

/**
 * \brief A widget to control a guideport
 */
class guideportcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::GuidePortPrx	_guideport;

	Ice::ObjectPtr	_guideport_callback;
	Ice::Identity	_guideport_identity;

	QTimer	_activationTimerRAplus;
	QTimer	_activationTimerRAminus;
	QTimer	_activationTimerDECplus;
	QTimer	_activationTimerDECminus;

	float	_activationtime;
	unsigned char	_active;
	float	_guiderate;
public:
	explicit guideportcontrollerwidget(QWidget *parent = 0);
	~guideportcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();

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
	void	updateActivation();
	void	radecCorrection(astro::RaDec,bool);
	void	activateClicked();
	void	activate(astro::camera::GuidePortActivation);
	void	deactivatedRAplus();
	void	deactivatedRAminus();
	void	deactivatedDECplus();
	void	deactivatedDECminus();
};

} // namespace snowogui

#endif // GUIDEPORTCONTROLLERWIDGET_H
