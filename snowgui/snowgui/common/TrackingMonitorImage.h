/*
 * TrackingMonitorImage.h -- class to display tracking images
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _TrackingMonitorImage_h
#define _TrackingMonitorImage_h

#include <MonitorImage.h>
#include <guider.h>

namespace snowgui {

/**
 * \brief MonitorImage designed to work with the Guider image callback
 */
class TrackingMonitorImage : public MonitorImage {
	snowstar::GuiderPrx	_guider;
	void	unregister();
	void	reregister();
public:
	TrackingMonitorImage(QObject *parent, QLabel *label);
	virtual ~TrackingMonitorImage();
	void	setGuider(snowstar::GuiderPrx guider, Ice::ObjectPtr myself);
};

} // namespace snowgui

#endif /* _TrackingMonitorImage_h */
