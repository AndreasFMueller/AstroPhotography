/*
 * mountcontrollerwidget.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef MOUNTCONTROLLERWIDGET_H
#define MOUNTCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <QTimer>
#include <AstroCoordinates.h>
#include <skydisplaydialog.h>
#include <catalogdialog.h>
#include <device.h>
#include <CommonClientTasks.h>
#include <CallbackIdentity.h>

namespace snowgui {

namespace Ui {
	class mountcontrollerwidget;
}

class mountcontrollerwidget;

/**
 * \brief A callback class for mount monitoring
 */
class MountCallbackI : public QObject, public snowstar::MountCallback,
			public CallbackIdentity {
	Q_OBJECT
public:
	MountCallbackI();
	void	statechange(snowstar::mountstate newstate,
			const Ice::Current& current);
	void	position(const snowstar::RaDec& newposition,
			const Ice::Current& current);

signals:
	void	callbackStatechange(snowstar::mountstate newstate);
	void	callbackPosition(snowstar::RaDec newposition);
};

/**
 * \brief Reusable component to control a telescope mount
 */
class mountcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::RaDec		_target;
	snowstar::mountstate	_previousstate;
	bool			_previouswest;
	snowstar::MountPrx	_mount;

	Ice::ObjectPtr		_mount_callback;

	snowstar::RaDec		_telescope;
	astro::LongLat		_location;
	SkyDisplayDialog	*_skydisplay;
	CatalogDialog		*_catalogdialog;


public:
	explicit mountcontrollerwidget(QWidget *parent = 0);
	~mountcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	astro::RaDec	current();
	void	setTarget(const astro::RaDec& target);

	bool	orientation();

signals:
	void	mountSelected(int);
	void	stateChanged(astro::device::Mount::state_type);
	void	telescopeChanged(astro::RaDec);
	void	retarget(astro::RaDec);
	void	orientationChanged(bool west);
	void	updateTime(time_t);
	void	radecCorrection(astro::RaDec,bool);

private:
	Ui::mountcontrollerwidget *ui;

	void	setupMount();
	void	targetChangedCommon();

public slots:
	void	mountChanged(int);
	void	gotoClicked();
	void	currentUpdate();
	void	statusUpdate();
	void	viewskyClicked();
	void	skyviewDestroyed();
	void	targetChanged(astro::RaDec);
	void	catalogClicked();
	void	catalogDestroyed();
	void	targetRaChanged(const QString&);
	void	targetDecChanged(const QString&);

	void	callbackStatechange(snowstar::mountstate newstate);
	void	callbackPosition(snowstar::RaDec newposition);
};

} // namespace snowgui

#endif // MOUNTCONTROLLERWIDGET_H
