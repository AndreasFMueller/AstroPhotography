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

namespace snowgui {

namespace Ui {
	class flatwidget;
}

class flatwidget : public QDialog {
	Q_OBJECT

	astro::image::ImagePtr	_flatimage;
	QTimer		statusTimer;
	snowstar::GuiderPrx	_guider;
	snowstar::GuiderState	_guiderstate;
	bool	_acquiring;
public:
	explicit flatwidget(QWidget *parent = 0);
	~flatwidget();

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
	Ui::flatwidget *ui;
};

} // namespace snowgui

#endif // SNOWGUI_FLATWIDGET_H
