/*
 * GuiderI.h -- guider servant definition
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderI_h
#define _GuiderI_h

#include <guider.h>
#include <AstroGuiding.h>
#include <ImageDirectory.h>
#include <AstroPersistence.h>

namespace snowstar {

GuiderState	convert(const astro::guiding::GuiderState& state);

class GuiderI : public Guider {
	astro::guiding::GuiderPtr	guider;
	astro::image::ImageDirectory	imagedirectory;
	astro::persistence::Database	database;
	Point	_point;
	int	calibrationid;
	int	guidingrunid;
	astro::guiding::TrackerPtr	getTracker();
public:
	GuiderI(astro::guiding::GuiderPtr _guider,
		astro::image::ImageDirectory& imagedirectoy,
		astro::persistence::Database database);
	virtual ~GuiderI();
	virtual GuiderState getState(const Ice::Current& current);
	virtual CameraPrx getCamera(const Ice::Current& current);
	virtual CcdPrx getCcd(const Ice::Current& current);
	virtual GuiderPortPrx getGuiderPort(const Ice::Current& current);
	virtual GuiderDescriptor getDescriptor(const Ice::Current& current);
	virtual void setExposure(const Exposure&, const Ice::Current& current);
	virtual Exposure getExposure(const Ice::Current& current);
	virtual void setStar(const Point&, const Ice::Current& current);
	virtual Point getStar(const Ice::Current& current);
	virtual void useCalibration(Ice::Int, const Ice::Current& current);
	virtual Calibration getCalibration(const Ice::Current& current);
	virtual void startCalibration(Ice::Float, const Ice::Current& current);
	virtual Ice::Double calibrationProgress(const Ice::Current& current);
	virtual void cancelCalibration(const Ice::Current& current);
	virtual bool waitCalibration(Ice::Double, const Ice::Current& current);
	virtual void startGuiding(Ice::Float, const Ice::Current& current);
	virtual Ice::Float getGuidingInterval(const Ice::Current& current);
	virtual void stopGuiding(const Ice::Current& current);
	virtual ImagePrx mostRecentImage(const Ice::Current& current);
	virtual TrackingPoint mostRecentTrackingPoint(const Ice::Current& current);
	virtual TrackingHistory getTrackingHistory(Ice::Int, const Ice::Current& current);

	virtual void	registerImageMonitor(
				const Ice::Identity& imagecallback,
				const Ice::Current& current);
	virtual void	unregisterImageMonitor(
				const Ice::Identity& imagecallback,
				const Ice::Current& current);
	virtual void	registerTrackingMonitor(
				const Ice::Identity& trackingcallback,
				const Ice::Current& current);
	virtual void	unregisterTrackingMonitor(
				const Ice::Identity& trackingcallback,
				const Ice::Current& current);
};

} // namespace snowstar

#endif /* _GuiderI_h */
