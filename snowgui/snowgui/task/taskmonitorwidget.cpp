/*
 * taskmonitorwidget.cpp -- widget to monitor task changes
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "taskmonitorwidget.h"
#include "ui_taskmonitorwidget.h"
#include <CommunicatorSingleton.h>
#include <IceConversions.h>

namespace snowgui {

taskmonitorwidget::taskmonitorwidget(QWidget *parent) : QWidget(parent),
	    ui(new Ui::taskmonitorwidget) {
	ui->setupUi(this);

	// set the table headers
	QStringList	headerlist;
	headerlist << "Time" << "Task" << "State";
	ui->monitorTable->setHorizontalHeaderLabels(headerlist);
	ui->monitorTable->horizontalHeader()->setStretchLastSection(true);

	// make first columns somewhat smaller
	ui->monitorTable->setColumnWidth(0, 150);
	ui->monitorTable->setColumnWidth(1, 50);
}

taskmonitorwidget::~taskmonitorwidget() {
	if (_taskmonitor) {
		Ice::Identity	identity = _taskmonitor->identity();
		snowstar::CommunicatorSingleton::remove(identity);
	}
	delete ui;
}

void	taskmonitorwidget::setServiceObject(
		astro::discover::ServiceObject serviceobject) {
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
        astro::ServerName       servername(serviceobject.name());
        Ice::ObjectPrx  base
                = ic->stringToProxy(servername.connect("Tasks"));
	_tasks = snowstar::TaskQueuePrx::checkedCast(base);

	if (!_tasks) {
		return;
	}

	_taskmonitor = new TaskMonitorController(NULL);
	_taskmonitorptr = Ice::ObjectPtr(_taskmonitor);
	_taskmonitor->setTasks(_tasks, _taskmonitorptr);

	// connect the task monitor to this widget
	connect(_taskmonitor, SIGNAL(taskUpdate(snowstar::TaskMonitorInfo)),
		this, SLOT(taskUpdate(snowstar::TaskMonitorInfo)));
}

void	taskmonitorwidget::taskUpdate(snowstar::TaskMonitorInfo info) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a new taskinfo: %d, state %d",
		info.taskid, info.newstate);
	QTableWidgetItem	*i;

	// increase the row count
	int	row = ui->monitorTable->rowCount();
	ui->monitorTable->setRowCount(row + 1);
	ui->monitorTable->setRowHeight(row, 15);

	// entry with the time
	time_t	when = snowstar::converttime(info.timeago);
	struct tm	*tmp = localtime(&when);
        char	buffer[100];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	i = new QTableWidgetItem(buffer);
	i->setFlags(Qt::NoItemFlags);
	ui->monitorTable->setItem(row, 0, i);

	// entry for the task number
	i = new QTableWidgetItem(QString::number(info.taskid));
	i->setFlags(Qt::NoItemFlags);
	i->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
	ui->monitorTable->setItem(row, 1, i);

	// entry for the state
	switch (info.newstate) {
	case snowstar::TskPENDING:
		i = new QTableWidgetItem("pending");
		break;
	case snowstar::TskEXECUTING:
		i = new QTableWidgetItem("executing");
		break;
	case snowstar::TskFAILED:
		i = new QTableWidgetItem("failed");
		break;
	case snowstar::TskCANCELLED:
		i = new QTableWidgetItem("cancelled");
		break;
	case snowstar::TskCOMPLETE:
		i = new QTableWidgetItem("complete");
		break;
	}
	i->setFlags(Qt::NoItemFlags);
	ui->monitorTable->setItem(row, 2, i);
	ui->monitorTable->scrollToBottom();
}

} // namespace snowgui
