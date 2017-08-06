/*
 * flatwidget.h -- widget to control generation of a new flat image
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_FLATWIDGET_H
#define SNOWGUI_FLATWIDGET_H

#include <QDialog>
#include <AstroImage.h>
#include <image.h>
#include <guider.h>
#include <QTimer>
#include <imagedisplaywidget.h>
#include "calibrationimagewidget.h"

namespace snowgui {

namespace Ui {
	class flatwidget;
}

class flatwidget : public calibrationimagewidget {
	Q_OBJECT

public:
	explicit flatwidget(QWidget *parent = 0);
	virtual ~flatwidget();

	void	exposuretime(double e);
	virtual void	checkImage();
	virtual std::string	imagetype() { return std::string("flat"); }

public slots:
	void	statusUpdate();
	void	acquireClicked();
	void	signalUpdated(snowstar::CalibrationImageProgress);
	void	stopped();

private:
	Ui::flatwidget *ui;
};

} // namespace snowgui

#endif // SNOWGUI_FLATWIDGET_H
