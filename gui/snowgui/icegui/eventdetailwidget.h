/*
 * eventdetailwidget.h
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_EVENTDETAILWIDGET_H
#define SNOWGUI_EVENTDETAILWIDGET_H

#include <QWidget>
#include <types.h>

namespace snowgui {

namespace Ui {
	class EventDetailWidget;
}

class EventDetailWidget : public QWidget {
	Q_OBJECT

public:
	explicit EventDetailWidget(QWidget *parent = 0);
	~EventDetailWidget();

	void	setEvent(const snowstar::Event& event);

private:
	Ui::EventDetailWidget *ui;
};

} // namespace snowgui
#endif // SNOWGUI_EVENTDETAILWIDGET_H
