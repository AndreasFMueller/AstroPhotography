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
#include <HeartbeatMonitor.h>
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace snowgui {

namespace Ui {
	class SystemInfoWidget;
}

/**
 * \brief A Widget to display systeminfo, including heartbeat information
 */
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

	/**
	 * \brief Add a receiver
	 *
	 * This method assumes that the receiver class has the methods
	 * heartbeat_update, heartbeat_lost, heartbeat_reconnected
	 *
	 * \param r	the receiver to add
	 */
	template<typename receiver>
	void	add_receiver(receiver *r) {
		HeartbeatMonitor	*h
			= dynamic_cast<HeartbeatMonitor*>(&*_heartbeat_monitor);
		if (NULL == h) {
			return;
		}
		// add connection from the heartbeat monitor
		connect(h, SIGNAL(update(QString)),
			r, SLOT(heartbeat_update(QString)),
			Qt::QueuedConnection);
		connect(h, SIGNAL(lost()),
			r, SLOT(heartbeat_lost()),
			Qt::QueuedConnection);
		connect(h, SIGNAL(reconnected()),
			r, SLOT(heartbeat_reconnected()),
			Qt::QueuedConnection);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new receiver %s connected",
			astro::demangle(typeid(*r).name()).c_str());
	}

	static SystemInfoWidget	*global();
	static void	setGlobal(SystemInfoWidget *s);

public slots:
	void	update();
	void	heartbeat_update(QString);
	void	heartbeat_lost();
	void	heartbeat_reconnected();
};

} // namespace snowgui

#endif // SNOWGUI_SYSTEMINFOWIDGET_H
