/*
 * taskwindow.h -- widget to control tasks
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef TASKWINDOW_H
#define TASKWINDOW_H

#include <InstrumentWidget.h>
#include "TaskMonitorController.h"

namespace snowgui {

namespace Ui {
	class taskwindow;
}

class taskwindow : public snowgui::InstrumentWidget {
	Q_OBJECT

public:
	explicit taskwindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~taskwindow();

private:
	Ui::taskwindow *ui;
signals:
	void	imageReceived(astro::image::ImagePtr);
public slots:
	void	receiveImage(astro::image::ImagePtr image);
	void	rectangleSelected(astro::image::ImageRectangle);
protected:
	void    closeEvent(QCloseEvent *);
};

} // namespace snowgui

#endif // TASKWINDOW_H
