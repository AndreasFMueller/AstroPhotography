/*
 * pointingwindow.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_POINTINGWINDOW_H
#define SNOWGUI_POINTINGWINDOW_H

#include <InstrumentWidget.h>

namespace snowgui {

namespace Ui {
	class pointingwindow;
}

class pointingwindow : public InstrumentWidget {
	Q_OBJECT

public:
	explicit pointingwindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~pointingwindow();

public slots:
	void	newImage(astro::image::ImagePtr image);
	void	pointSelected(astro::image::ImagePoint);

private:
	Ui::pointingwindow *ui;

protected:
	void	closeEvent(QCloseEvent *);
};

} // namespace snowgui
#endif // SNOWGUI_POINTINGWINDOW_H
