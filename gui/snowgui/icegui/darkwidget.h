/*
 * darkwidget.h -- widget to control generation of a new dark image
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_DARKWIDGET_H
#define SNOWGUI_DARKWIDGET_H

#include <QDialog>
#include <AstroImage.h>
#include <image.h>
#include <guider.h>
#include <QTimer>
#include <imagedisplaywidget.h>
#include "calibrationimagewidget.h"

namespace snowgui {

namespace Ui {
	class darkwidget;
}

class darkwidget : public calibrationimagewidget {
	Q_OBJECT

public:
	explicit darkwidget(QWidget *parent = 0);
	virtual ~darkwidget();

	void	exposuretime(double e);
	virtual void	checkImage();
	virtual std::string 	imagetype() { return std::string("dark"); }

public slots:
	void	statusUpdate();
	void	acquireClicked();
	void	signalUpdated(snowstar::CalibrationImageProgress);
	void	stopped();

private:
	Ui::darkwidget *ui;
};

} // namespace snowgui

#endif // SNOWGUI_DARKWIDGET_H
