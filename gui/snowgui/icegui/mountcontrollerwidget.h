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
#include <SkyDisplayWidget.h>

namespace snowgui {

namespace Ui {
	class mountcontrollerwidget;
}

class mountcontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::RaDec		_target;
	snowstar::mountstate	_previousstate;
	snowstar::MountPrx	_mount;

	snowstar::RaDec		_position;
	SkyDisplayWidget	*_skydisplay;

public:
	explicit mountcontrollerwidget(QWidget *parent = 0);
	~mountcontrollerwidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	astro::RaDec	current();
	void	setTarget(const astro::RaDec& target);

signals:
	void	mountSelected(int);
	void	positionChanged(astro::RaDec);

private:
	Ui::mountcontrollerwidget *ui;
	QTimer	_statusTimer;

	void	setupMount();

public slots:
	void	mountChanged(int);
	void	gotoClicked();
	void	statusUpdate();
	void	viewskyClicked();
};

} // namespace snowgui

#endif // MOUNTCONTROLLERWIDGET_H
