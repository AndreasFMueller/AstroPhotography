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

class SystemInfoWidget : public QWidget {
	Q_OBJECT

	QTimer	_timer;
	snowstar::DaemonPrx     _daemon;
	Ui::SystemInfoWidget *ui;
public:
	explicit SystemInfoWidget(QWidget *parent = nullptr);
	~SystemInfoWidget();

	void	setDaemon(snowstar::DaemonPrx daemon);

public slots:
	void	update();
};

} // namespace snowgui

#endif // SNOWGUI_SYSTEMINFOWIDGET_H
