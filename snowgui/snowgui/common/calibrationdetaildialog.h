/*
 * calibrationdetaildialog.h -- detail view of a calibration
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_CALIBRATIONDETAILDIALOG_H
#define SNOWGUI_CALIBRATIONDETAILDIALOG_H

#include <QDialog>
#include <guider.h>

namespace snowgui {

namespace Ui {
	class calibrationdetaildialog;
}

class calibrationdetaildialog : public QDialog {
	Q_OBJECT

	snowstar::Calibration	_calibration;

public:
	explicit calibrationdetaildialog(QWidget *parent = 0);
	~calibrationdetaildialog();

public slots:
	void	setCalibration(snowstar::Calibration calibration);

private:
	Ui::calibrationdetaildialog *ui;
};


} // namespace snowgui
#endif // SNOWGUI_CALIBRATIONDETAILDIALOG_H
