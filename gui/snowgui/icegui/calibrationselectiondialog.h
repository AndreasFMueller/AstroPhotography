/*
 * calibrationselectiondialog.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef CALIBRATIONSELECTIONDIALOG_H
#define CALIBRATIONSELECTIONDIALOG_H

#include <QDialog>
#include <guider.h>

namespace snowgui {

namespace Ui {
	class calibrationselectiondialog;
}

class calibrationselectiondialog : public QDialog
{
	Q_OBJECT

	snowstar::ControlType		_controltype;
	std::string			_instrumentname;
	snowstar::GuiderFactoryPrx	_guiderfactory;

	std::vector<snowstar::Calibration>	_calibrations;
	snowstar::Calibration	_calibration;
public:
	void	setGuider(snowstar::ControlType controltype,
			const std::string& instrumentname,
			snowstar::GuiderFactoryPrx guiderfactory);

public:
	explicit calibrationselectiondialog(QWidget *parent = 0);
	~calibrationselectiondialog();

signals:
	void	calibrationSelected(snowstar::Calibration);

private:
	Ui::calibrationselectiondialog *ui;

public slots:
	void	currentRowChanged(int);
	void	calibrationAccepted();
};

} // namespace snowgui

#endif // CALIBRATIONSELECTIONDIALOG_H
