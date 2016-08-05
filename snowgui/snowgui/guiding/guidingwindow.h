/*
 * guidingwindow.h -- main window for guiding subapplication
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef GUIDINGWINDOW_H
#define GUIDINGWINDOW_H

#include <InstrumentWidget.h>

namespace Ui {
	class guidingwindow;
}

namespace snowgui {

class guidingwindow : public InstrumentWidget {
	Q_OBJECT

public:
	explicit guidingwindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~guidingwindow();

private:
	Ui::guidingwindow *ui;
};

} // namespace snowgui

#endif // GUIDINGWINDOW_H
