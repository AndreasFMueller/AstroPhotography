/*
 * BacklashMonitor.h -- A monitor class to handle backlash updates
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _BacklashMonitor_h
#define _BacklashMonitor_h

#include <QObject>
#include <guider.h>

namespace snowgui {

class BacklashDialog;

class BacklashMonitor : public QObject, public snowstar::BacklashMonitor {
	Q_OBJECT
	BacklashDialog	*_backlashdialog;
public:
	BacklashMonitor(BacklashDialog *);

	virtual void	updatePoint(const snowstar::BacklashPoint& point,
				const Ice::Current& current);
	virtual void	updateResult(const snowstar::BacklashResult& result,
				const Ice::Current& current);
	virtual void	stop(const Ice::Current& current);
signals:
	void	updatePointSignal(snowstar::BacklashPoint);
	void	updateResultSignal(snowstar::BacklashResult);
	void	stopSignal();
};

} // namespace snowgui

#endif /* _BacklashMonitor_h */
