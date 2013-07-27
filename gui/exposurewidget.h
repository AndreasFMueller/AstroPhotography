/*
 * exposurewidget.h -- Widget to control exposure
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef EXPOSUREWIDGET_H
#define EXPOSUREWIDGET_H

#include <QGroupBox>
#include <AstroCamera.h>

using namespace astro::camera;

namespace Ui {
class ExposureWidget;
}

class ExposureWidget : public QGroupBox
{
    Q_OBJECT
	CcdPtr	ccd;

	// state variable to handle exponential time spinning
	bool	timechange;
	double	timeprevious;
    
public:
    explicit ExposureWidget(QWidget *parent = 0);
    ~ExposureWidget();

	void	setCcd(CcdPtr ccd);
	Exposure	getExposure();
	void	setExposure(const Exposure& exposure);
    
private:
    Ui::ExposureWidget *ui;

private slots:
	void	timeChanged(double value);
	void	subframeToggled(bool state);
};

#endif // EXPOSUREWIDGET_H
