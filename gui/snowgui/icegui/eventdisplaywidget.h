/*
 * eventdisplaywidget.h -- event display widget
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_EVENTDISPLAYWIDGET_H
#define SNOWGUI_EVENTDISPLAYWIDGET_H

#include <QWidget>
#include <AstroDiscovery.h>
#include <types.h>
#include <QTableWidgetItem>
#include "eventdetailwidget.h"

namespace snowgui {

namespace Ui {
	class EventDisplayWidget;
}

class EventDisplayWidget : public QWidget {
	Q_OBJECT

	Ui::EventDisplayWidget *ui;
	snowstar::EventHandlerPrx	_events;
	astro::discover::ServiceObject	_serviceobject;
	Ice::Identity	_monitoridentity;
	EventDetailWidget	*_detailwidget;
public:
	explicit EventDisplayWidget(QWidget *parent,
		astro::discover::ServiceObject serviceobject);
	~EventDisplayWidget();

	void	setEvents(snowstar::EventHandlerPrx events);
	void	addPastEvents();
	void	clearEvents();
	void	insertEvent(int row, const snowstar::Event& event);
	void	closeEvent(QCloseEvent *);

public slots:
	void	filterEdited(const QString&);
	void	timeSelectClicked();
	void	filterClicked();
	void	stopSignaled();
	void	updateSignaled(const snowstar::Event& event);
	void	eventDoubleClicked(QTableWidgetItem*);
};

} // namespace snowgui

#endif // SNOWGUI_EVENTDISPLAYWIDGET_H
