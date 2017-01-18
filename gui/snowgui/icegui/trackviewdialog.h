/*
 * trackviewdialog.h -- dialog to view a track
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TRACKVIEWDIALOG_H
#define SNOWGUI_TRACKVIEWDIALOG_H

#include <QDialog>
#include <guider.h>

namespace snowgui {

namespace Ui {
	class trackviewdialog;
}

class trackviewdialog : public QDialog {
	Q_OBJECT

	snowstar::TrackingHistory	_track;
	snowstar::GuiderFactoryPrx	_guiderfactory;
	snowstar::Calibration	_gpcalibration;
	snowstar::Calibration	_aocalibration;
public:
	explicit trackviewdialog(QWidget *parent = 0);
	~trackviewdialog();

	void	setGuiderFactory(snowstar::GuiderFactoryPrx);

	typedef enum datatype_e { offsetPx, offsetArcsec, correction } datatype_t;
	datatype_t	_datatype;
	void	updateData();

public slots:
	void	setTrack(snowstar::TrackingHistory track);
	void	buttonToggled(bool);

private:
	Ui::trackviewdialog *ui;
};

} // namespace snowgui
#endif // SNOWGUI_TRACKVIEWDIALOG_H
