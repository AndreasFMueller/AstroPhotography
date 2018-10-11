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

namespace snowgui {

namespace Ui {
	class mountcontrollerwidget;
}

class mountcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::RaDec		_target;
	snowstar::mountstate	_previousstate;
	bool			_previouswest;
	snowstar::MountPrx	_mount;

	snowstar::RaDec		_telescope;
	astro::LongLat	_position;
	SkyDisplayDialog	*_skydisplay;

public:
	explicit mountcontrollerwidget(QWidget *parent = 0);
	~mountcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	astro::RaDec	current();
	void	setTarget(const astro::RaDec& target);

signals:
	void	mountSelected(int);
	void	stateChanged(astro::device::Mount::state_type);
	void	telescopeChanged(astro::RaDec);
	void	orientationChanged(bool west);
	void	updateTime(time_t);
	void	radecCorrection(astro::RaDec,bool);

private:
	Ui::mountcontrollerwidget *ui;
	QTimer	_statusTimer;

	void	setupMount();

public slots:
	void	mountChanged(int);
	void	gotoClicked();
	void	statusUpdate();
	void	viewskyClicked();
	void	skyviewDestroyed();
	void	targetChanged(astro::RaDec);
};

} // namespace snowgui

#endif // MOUNTCONTROLLERWIDGET_H
