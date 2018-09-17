/*
 * guidercontrollerwidget.h -- widget to control a guider
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDERCONTROLLERWIDGET_H
#define GUIDERCONTROLLERWIDGET_H

#include <InstrumentWidget.h>
#include <guider.h>
#include <AstroCamera.h>
#include <AstroImage.h>
#include <TrackingMonitorImage.h>
#include <TrackingMonitorController.h>
#include <QTimer>
#include "backlashdialog.h"

namespace snowgui {

namespace Ui {
	class guidercontrollerwidget;
}

class guidercontrollerwidget : public InstrumentWidget {
	Q_OBJECT

	snowstar::GuiderFactoryPrx	_guiderfactory;
	snowstar::GuiderDescriptor	_guiderdescriptor;
	snowstar::GuiderPrx		_guider;

	double	_gpupdateinterval;
	double	_aoupdateinterval;
	bool	_stepping;

	astro::camera::Exposure		_exposure;
	astro::image::ImagePoint	_star;
	double	_windowradius;

	snowstar::GuiderState	_previousstate;
	QTimer	statusTimer;

	TrackingMonitorImage	*_trackingmonitorimage;
	Ice::ObjectPtr	_trackingmonitorimageptr;
	QLabel	*_trackinglabel;

	TrackingMonitorController	*_trackingmonitor;
	Ice::ObjectPtr	_trackingmonitorptr;
	trackingmonitordialog	*_trackingmonitordialog;

	BacklashDialog	*_backlashDialog;

	void	setupGuider();

public:
	explicit guidercontrollerwidget(QWidget *parent = 0);
	virtual void    instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~guidercontrollerwidget();
	void	setupTracker();
	void	setupFilter();

signals:
	void	exposureChanged(astro::camera::Exposure);

private:
	Ui::guidercontrollerwidget *ui;

public slots:
	void	setExposure(astro::camera::Exposure);
	void	setStar(astro::image::ImagePoint);
	void	setCcd(int);
	void	setGuideport(int);
	void	setAdaptiveoptics(int);

	void	startGuiding();
	void	stopGuiding();
	void	selectTrack();
	void	trackSelected(snowstar::TrackingHistory track);

	void	selectPoint(astro::image::ImagePoint);
	void	trackingMethodChanged(int);
	void	filterMethodChanged(int);

	void	gpupdateintervalChanged(double);
	void	aoupdateintervalChanged(double);
	void	windowradiusChanged(double);

	void	statusUpdate();

	void	toggleFreeze(bool);
	void	toggleInverse(bool);
	void	launchMonitor();
	void	imageUpdated();

	void	backlashRAClicked();
	void	backlashDECClicked();

	void	xGainChanged(int);
	void	yGainChanged(int);
};

} // namespace snowgui

#endif // GUIDERCONTROLLERWIDGET_H
