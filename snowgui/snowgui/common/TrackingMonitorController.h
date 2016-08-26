/*
 * TrackingMonitorController.h -- controller class to manage tracking dialog
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _TrackingMonitorController_h
#define _TrackingMonitorController_h

#include <QObject>
#include <trackingmonitordialog.h>
#include <guider.h>

namespace snowgui {

class TrackingMonitorController : public QObject, public snowstar::TrackingMonitor {
	Q_OBJECT
	
	trackingmonitordialog	*_dialog;

	snowstar::GuiderPrx	_guider;

protected:
	Ice::Identity	_myidentity;
public:
	TrackingMonitorController(QObject *parent, trackingmonitordialog *dialog);
	virtual ~TrackingMonitorController();

	void    setGuider(snowstar::GuiderPrx proxy, Ice::ObjectPtr myself);

	// callback methods
	void	stop(const Ice::Current&);
	void	update(const snowstar::TrackingPoint&, const Ice::Current&);

signals:
	void	dataUpdated();

public slots:
	void	refreshDisplay();
};

} // namespace snowgui

#endif /* _TrackingMonitorController_h */
