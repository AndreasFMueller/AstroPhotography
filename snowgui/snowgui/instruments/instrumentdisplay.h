/*
 * instrumentdisplay.h
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_INSTRUMENTDISPLAY_H
#define SNOWGUI_INSTRUMENTDISPLAY_H

#include <QWidget>
#include <instruments.h>

namespace snowgui {

namespace Ui {
	class instrumentdisplay;
}

class instrumentdisplay : public QWidget {
	Q_OBJECT

public:
	explicit instrumentdisplay(QWidget *parent = 0);
	~instrumentdisplay();

	void	setInstrument(snowstar::InstrumentPrx instrument);

private:
	Ui::instrumentdisplay *ui;
	snowstar::InstrumentPrx	_instrument;
	void 	toplevel(snowstar::InstrumentComponentType type,
			const std::string& name);
	void	alltoplevel();
	void	children(snowstar::InstrumentComponentType type);
	void	allchildren();
};


} // namespace snowgui
#endif // SNOWGUI_INSTRUMENTDISPLAY_H
