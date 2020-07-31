/*
 * systeminfowidget.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_SYSTEMINFOWIDGET_H
#define SNOWGUI_SYSTEMINFOWIDGET_H

#include <QWidget>
#include <QTimer>
#include <types.h>

namespace snowgui {

namespace Ui {
	class SystemInfoWidget;
}

class HeartbeatMonitor : public QObject, public snowstar::HeartbeatMonitor {
	Q_OBJECT

public:
	HeartbeatMonitor();
	virtual ~HeartbeatMonitor();

	virtual void    beat(int sequence_number, 
				const Ice::Current& /* current */);
	virtual void    stop(const Ice::Current& /* current */);
	
signals:
	void	update(QString);
};

class SystemInfoWidget : public QWidget {
	Q_OBJECT

	QTimer	_timer;
	snowstar::DaemonPrx     _daemon;
	Ui::SystemInfoWidget *ui;
	Ice::Identity	_heartbeat_identity;
	Ice::ObjectPtr	_heartbeat_monitor;
public:
	explicit SystemInfoWidget(QWidget *parent = nullptr);
	~SystemInfoWidget();

	void	setDaemon(snowstar::DaemonPrx daemon);

public slots:
	void	update();
	void	heartbeat_update(QString);
};

} // namespace snowgui

#endif // SNOWGUI_SYSTEMINFOWIDGET_H
