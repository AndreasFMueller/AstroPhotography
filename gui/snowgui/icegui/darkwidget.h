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

namespace snowgui {

namespace Ui {
	class darkwidget;
}

class darkwidget : public QDialog {
	Q_OBJECT

	astro::image::ImagePtr	_darkimage;
	QTimer		statusTimer;
	snowstar::GuiderPrx	_guider;
	snowstar::GuiderState	_guiderstate;
	bool	_acquiring;
public:
	explicit darkwidget(QWidget *parent = 0);
	~darkwidget();

	void	guider(snowstar::GuiderPrx guider);

	void	closeEvent(QCloseEvent *);
	void	exposuretime(double e);
	void	checkImage();

signals:
	void	newImage(astro::image::ImagePtr);

public slots:
	void	statusUpdate();
	void	acquireClicked();
	void	viewClicked();

private:
	Ui::darkwidget *ui;
};

} // namespace snowgui

#endif // SNOWGUI_DARKWIDGET_H
