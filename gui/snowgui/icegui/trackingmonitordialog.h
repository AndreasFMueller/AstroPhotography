/*
 * trackingmonitordialog.h -- dialog to monitor the tracking accuracy
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TRACKINGMONITORDIALOG_H
#define SNOWGUI_TRACKINGMONITORDIALOG_H

#include <QDialog>
#include <guider.h>

namespace snowgui {

namespace Ui {
	class trackingmonitordialog;
}

class trackingmonitordialog : public QDialog {
	Q_OBJECT

public:
	explicit trackingmonitordialog(QWidget *parent = 0);
	~trackingmonitordialog();

	void	add(const snowstar::TrackingHistory& history);
	void	add(const snowstar::TrackingPoint& point);
	void	updateData();
	void	clearData();

	void	gpMasperpixel(double masperpixel);
	void	aoMasperpixel(double masperpixel);
private:
	Ui::trackingmonitordialog *ui;
};

} // namespace snowgui

#endif // SNOWGUI_TRACKINGMONITORDIALOG_H
