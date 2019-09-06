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

namespace snowgui {

namespace Ui {
	class mountcontrollerwidget;
}

class mountcontrollerwidget;

/**
 * \brief Update thread for the mount controller
 */
class	mountupdatework : public QObject {
	Q_OBJECT
	mountcontrollerwidget	*_mountcontrollerwidget;
	std::recursive_mutex	_mutex;
public:
	mountupdatework(mountcontrollerwidget *mc);
	~mountupdatework();
public slots:
	void	statusUpdate();
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

	snowstar::RaDec		_telescope;
	astro::LongLat		_position;
	SkyDisplayDialog	*_skydisplay;
	CatalogDialog		*_catalogdialog;

	mountupdatework	*_updatework;
	QThread		*_updatethread;

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
	QTimer	_statusTimer;

	void	setupMount();
	void	targetChangedCommon();

public slots:
	void	mountChanged(int);
	void	gotoClicked();
	void	statusUpdate();
	void	viewskyClicked();
	void	skyviewDestroyed();
	void	targetChanged(astro::RaDec);
	void	catalogClicked();
	void	catalogDestroyed();
	void	targetRaChanged(const QString&);
	void	targetDecChanged(const QString&);
};

} // namespace snowgui

#endif // MOUNTCONTROLLERWIDGET_H
