#ifndef SNOWGUI_EXPOSEWINDOW_H
#define SNOWGUI_EXPOSEWINDOW_H

#include <InstrumentWidget.h>

namespace snowgui {

namespace Ui {
	class exposewindow;
}

class exposewindow : public InstrumentWidget {
	Q_OBJECT

public:
	explicit exposewindow(QWidget *parent = 0);
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	~exposewindow();

private:
	Ui::exposewindow *ui;
protected:
	void	closeEvent(QCloseEvent *);
};


} // namespace snowgui
#endif // SNOWGUI_EXPOSEWINDOW_H
