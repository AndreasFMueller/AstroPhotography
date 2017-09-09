/*
 * eventdetailwidget.cpp -- 
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "eventdetailwidget.h"
#include "ui_eventdetailwidget.h"
#include <IceConversions.h>

namespace snowgui {

/**
 * \brief Construct a new eventdetail widget
 */
EventDetailWidget::EventDetailWidget(QWidget *parent) : QWidget(parent),
	ui(new Ui::EventDetailWidget) {
	ui->setupUi(this);
}

/**
 * \brief Destroy the event detail widget
 */
EventDetailWidget::~EventDetailWidget() {
	delete ui;
}

/**
 * \brief display an event
 */
void	EventDetailWidget::setEvent(const snowstar::Event& event) {
	std::string	levelstring
		= astro::events::level2string(snowstar::convert(event.level));
	ui->levelField->setText(levelstring.c_str());

	ui->serviceField->setText(event.service.c_str());
	ui->pidField->setText(astro::stringprintf("%d", event.pid).c_str());

	ui->subsystemField->setText(event.subsystem.c_str());

	struct timeval	when = snowstar::converttimeval(event.timeago);
	std::string	timestamp = astro::Timer::timestamp(when, 3);
	ui->timeField->setText(timestamp.c_str());

	ui->messageField->setText(event.message.c_str());

	ui->classnameField->setText(event.classname.c_str());

	ui->fileField->setText(event.file.c_str());
	ui->lineField->setText(astro::stringprintf("%d", event.line).c_str());

	ui->subsystemField->setText(event.subsystem.c_str());

	std::string	title = astro::stringprintf("%s Event %d @ %s",
			levelstring.c_str(), event.id, timestamp.c_str());
	setWindowTitle(QString(title.c_str()));
}

} // namespace snowgui
