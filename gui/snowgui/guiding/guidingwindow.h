/*
 * guidingwindow.h -- main window for guiding subapplication
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDINGWINDOW_H
#define GUIDINGWINDOW_H

#include <InstrumentWidget.h>
#include <AstroImage.h>

namespace snowgui {

namespace Ui {
	class guidingwindow;
}

class guidingwindow : public InstrumentWidget {
	Q_OBJECT

public:
	explicit guidingwindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~guidingwindow();

public slots:
	void	newImage(astro::image::ImagePtr image);

private:
	Ui::guidingwindow *ui;

protected:
	void	closeEvent(QCloseEvent *);
};

} // namespace snowgui

#endif // GUIDINGWINDOW_H
