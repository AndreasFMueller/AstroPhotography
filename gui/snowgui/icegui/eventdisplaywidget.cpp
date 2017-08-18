/*
 * eventdisplaywidget.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "eventdisplaywidget.h"
#include "ui_eventdisplaywidget.h"
#include <CommunicatorSingleton.h>
#include <QTableWidget>
#include <IceConversions.h>
#include <time.h>
#include <EventMonitor.h>
#include <AstroUtils.h>

namespace snowgui {

/**
 * \brief Construct an Event display widget
 */
EventDisplayWidget::EventDisplayWidget(QWidget *parent,
	astro::discover::ServiceObject serviceobject)
	: QWidget(parent), ui(new Ui::EventDisplayWidget),
	  _serviceobject(serviceobject) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating EventDisplay %s",
		_serviceobject.toString().c_str());
	ui->setupUi(this);

	// connect the widgets
	debug(LOG_DEBUG, DEBUG_LOG, 0, "connect fields");
	connect(ui->filterButton, SIGNAL(clicked()),
		this, SLOT(filterClicked()));
	connect(ui->timeSelectButton, SIGNAL(clicked()),
		this, SLOT(timeSelectClicked()));

	// configure the table
	debug(LOG_DEBUG, DEBUG_LOG, 0, "configure table");
	QStringList	headers;
	headers << "Level";
	headers << "PID";
	headers << "service";
	headers << "time";
	headers << "subsystem";
	headers << "message";
	headers << "classname";
	headers << "file";
	headers << "line";
        ui->eventTable->setHorizontalHeaderLabels(headers);

	// window title
	std::string	title = astro::stringprintf("Events on %s",
		_serviceobject.toString().c_str());
	this->setWindowTitle(QString(title.c_str()));

	// time span
	time_t	now;
	time(&now);
	QDateTime	fromdatetime;
	fromdatetime.setTime_t(now - 3600);
	ui->fromTime->setDateTime(fromdatetime);
	QDateTime	todatetime;
	todatetime.setTime_t(now + 3600);
	ui->toTime->setDateTime(todatetime);

	// connect to the event handler
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
		_serviceobject.connect("Events"));
	if (!base) {
		throw std::runtime_error("cannot create events app");
	}
	snowstar::EventHandlerPrx	events
		= snowstar::EventHandlerPrx::checkedCast(base);
	setEvents(events);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "event display constructed");
}

/**
 * \brief Destroy the event display widget
 */
EventDisplayWidget::~EventDisplayWidget() {
	if (_events) {
		try {
			_events->unregisterMonitor(_monitoridentity);
		} catch (const std::exception& x) {
		}
	}
	delete ui;
}

/**
 * \brief set the event handler interface for this widget
 */
void	EventDisplayWidget::setEvents(snowstar::EventHandlerPrx events) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "install a new event handler proxy");
	// unregister an event handler that might still be present
	if (_events) {
		try {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "unregistering monitor "
				"%s", _monitoridentity.name.c_str());
			_events->unregisterMonitor(_monitoridentity);
		} catch (const std::exception& x) {
		}
	}

	// switch to the new event handler
	_events = events;
	if (!_events) {
		return;
	}
	addPastEvents();

	// create a monitor an register it
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "creating the monitor");
		EventMonitor	*_monitor = new EventMonitor();
		snowstar::CommunicatorSingleton::connect(events);
		Ice::ObjectPtr	_monitorptr(_monitor);
		_monitoridentity = snowstar::CommunicatorSingleton::add(_monitorptr);
		_events->registerMonitor(_monitoridentity);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "register as %s",
			_monitoridentity.name.c_str());

		connect(_monitor, SIGNAL(stopSignal()),
			this, SLOT(stopSignaled()));
		connect(_monitor, SIGNAL(updateSignal(snowstar::Event)),
			this, SLOT(updateSignaled(snowstar::Event)));
	} catch (const std::exception& x) {
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new event handler proxy installed");
}

/**
 * \brief add an event to the tabel
 *
 * \param row		row where the event should be inserted
 * \param event		event to add
 */
void	EventDisplayWidget::insertEvent(int row, const snowstar::Event& event) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add event row %d", row);
	ui->eventTable->setRowHeight(row, 19);

	QTableWidgetItem	*item;

	std::string	levelstring = astro::events::level2string(snowstar::convert(event.level));
	item = new QTableWidgetItem(levelstring.c_str());
	ui->eventTable->setItem(row, 0, item);

	item = new QTableWidgetItem(astro::stringprintf("%d", event.pid).c_str());
	ui->eventTable->setItem(row, 1, item);

	item = new QTableWidgetItem(event.service.c_str());
	ui->eventTable->setItem(row, 2, item);


	struct timeval	when = snowstar::converttimeval(event.timeago);
	std::string	timestamp = astro::Timer::timestamp(when, 3);
	item = new QTableWidgetItem(timestamp.c_str());
	QFont   f("Microsoft Sans Serif");;
        f.setStyleHint(QFont::Monospace);
        item->setFont(f);
	ui->eventTable->setItem(row, 3, item);

	item = new QTableWidgetItem(event.subsystem.c_str());
	ui->eventTable->setItem(row, 4, item);

	item = new QTableWidgetItem(event.message.c_str());
	ui->eventTable->setItem(row, 5, item);

	item = new QTableWidgetItem(event.classname.c_str());
	ui->eventTable->setItem(row, 6, item);

	item = new QTableWidgetItem(event.file.c_str());
	ui->eventTable->setItem(row, 7, item);

	item = new QTableWidgetItem(astro::stringprintf("%d", event.line).c_str());
	item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ui->eventTable->setItem(row, 8, item);
}

/**
 * \brief add events from the past
 */
void	EventDisplayWidget::addPastEvents() {
	try {
		time_t	now;
		time(&now);
		snowstar::eventlist	list
			= _events->eventsBetween(
				now - ui->fromTime->dateTime().toTime_t(),
				now - ui->toTime->dateTime().toTime_t());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got %d events", list.size());
		ui->eventTable->setRowCount(list.size());
		EventDisplayWidget	*myself = this;
		int	row = 0;
		std::for_each(list.begin(), list.end(),
			[&row,myself](const snowstar::Event& event) mutable {
				myself->insertEvent(row, event);
				row++;
			}
		);

	} catch (const std::exception& x) {
		std::string	msg = astro::stringprintf(
			"cannot get past events: %s", x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	}
	ui->eventTable->resizeColumnsToContents();
}

/**
 * \brief remove all events from the table
 */
void	EventDisplayWidget::clearEvents() {
	ui->eventTable->setRowCount(0);
}

/**
 * \brief when the filter changes
 */
void	EventDisplayWidget::filterEdited(const QString& filter) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filter changed to %s",
		filter.toLatin1().data());
}

/**
 * \brief Slot to handle the stop message from the monitor
 */
void	EventDisplayWidget::stopSignaled() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stop event monitor");
}

/**
 * \brief slot to add new events
 */
void	EventDisplayWidget::updateSignaled(const snowstar::Event& event) {
	int	row = ui->eventTable->rowCount();
	ui->eventTable->setRowCount(row + 1);
	insertEvent(row, event);
	ui->eventTable->resizeColumnsToContents();
}

/**
 * \brief time select clicked
 */
void	EventDisplayWidget::timeSelectClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "timeSelectClicked");
	addPastEvents();
}

/**
 * \brief filter clicked
 */
void	EventDisplayWidget::filterClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterClicked");
}

} // namespace snowgui
