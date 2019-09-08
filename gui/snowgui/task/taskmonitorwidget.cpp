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
	headerlist << "Time" << "Task" << "Type" << "State";
	ui->monitorTable->setHorizontalHeaderLabels(headerlist);
	ui->monitorTable->horizontalHeader()->setStretchLastSection(true);

	// make first columns somewhat smaller
	ui->monitorTable->setColumnWidth(0, 150);
	ui->monitorTable->setColumnWidth(1, 40);
	ui->monitorTable->setColumnWidth(2, 70);
	//ui->monitorTable->setColumnWidth(3, 70);
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set service object");
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
        astro::ServerName       servername(serviceobject.name());
        Ice::ObjectPrx  base = ic->stringToProxy(servername.connect("Tasks"));
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set service object complete");
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

	// type info
	i = new QTableWidgetItem(QString(
		snowstar::tasktype2string(info.type).c_str()));
	i->setFlags(Qt::NoItemFlags);
	i->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	ui->monitorTable->setItem(row, 2, i);

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
	case snowstar::TskDELETED:
		i = new QTableWidgetItem("deleted");
		break;
	}
	i->setFlags(Qt::NoItemFlags);
	ui->monitorTable->setItem(row, 3, i);
	ui->monitorTable->scrollToBottom();
}

} // namespace snowgui
