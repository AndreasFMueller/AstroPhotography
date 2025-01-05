/*
 * taskqueuemanagerwidget.cpp -- task queue manager implementation
 *
 * (c) 2016 Prof Dr Andreas Müeller, Hochschule Rapperswil
 */
#include "taskqueuemanagerwidget.h"
#include "ui_taskqueuemanagerwidget.h"
#include <CommunicatorSingleton.h>
#include <AstroDebug.h>
#include <AstroCamera.h>
#include <IceConversions.h>

namespace snowgui {

#define	taskcol_id		 0
#define taskcol_type		 1
#define taskcol_instrument	 2
#define taskcol_project		 3
#define taskcol_purpose		 4
#define taskcol_lastchange	 5
#define taskcol_exposure	 6
#define taskcol_gain	 	 7
#define taskcol_filter		 8
#define taskcol_frame		 9
#define taskcol_binning		10
#define taskcol_temperature	11
#define taskcol_repository	12
#define taskcol_database	13
#define taskcol_filename	14

taskqueuemanagerwidget::taskqueuemanagerwidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::taskqueuemanagerwidget) {
	ui->setupUi(this);
	ui->taskTree->setSelectionMode(QAbstractItemView::ExtendedSelection);

	qRegisterMetaType<QVector<int>>("QVector<int>");

	ui->infoButton->setEnabled(false);
	ui->cancelButton->setEnabled(false);
	ui->deleteButton->setEnabled(false);
	ui->resubmitButton->setEnabled(false);
	ui->previewButton->setEnabled(false);
	ui->imageButton->setEnabled(false);
	ui->downloadButton->setEnabled(false);

	_taskinfowidget = NULL;
	_taskmonitor = NULL;

	// set the task times to zero
	_totaltimes.insert(std::make_pair(snowstar::TskPENDING,   (double)0.0));
	_totaltimes.insert(std::make_pair(snowstar::TskEXECUTING, (double)0.0));
	_totaltimes.insert(std::make_pair(snowstar::TskFAILED,    (double)0.0));
	_totaltimes.insert(std::make_pair(snowstar::TskCANCELLED, (double)0.0));
	_totaltimes.insert(std::make_pair(snowstar::TskCOMPLETE,  (double)0.0));

	// configure the task list
	QStringList	headers;
	headers << "ID";		//  0
	headers << "Type";		//  1
	headers << "Instrument";	//  2
	headers << "Project";		//  3
	headers << "Purpose";		//  4
	headers << "Last change";	//  5
	headers << "Exposure";		//  6
	headers << "Gain";		//  7
	headers << "Filter";		//  8
	headers << "Frame";		//  9
	headers << "Binning";		// 10
	headers << "Temperature";	// 11
	headers << "Repository";	// 12
	headers << "Database";		// 13
	headers << "Filename/Cause";	// 14
	ui->taskTree->setHeaderLabels(headers);
	ui->taskTree->header()->resizeSection(taskcol_id,          80);
	ui->taskTree->header()->resizeSection(taskcol_type,        80);
	ui->taskTree->header()->resizeSection(taskcol_instrument, 110);
	ui->taskTree->header()->resizeSection(taskcol_project,    100);
	ui->taskTree->header()->resizeSection(taskcol_purpose,     60);
	ui->taskTree->header()->resizeSection(taskcol_lastchange, 150);
	ui->taskTree->header()->resizeSection(taskcol_exposure,    60);
	ui->taskTree->header()->resizeSection(taskcol_gain,        60);
	ui->taskTree->header()->resizeSection(taskcol_filter,     100);
	ui->taskTree->header()->resizeSection(taskcol_frame,       90);
	ui->taskTree->header()->resizeSection(taskcol_binning,     50);
	ui->taskTree->header()->resizeSection(taskcol_temperature, 80);
	ui->taskTree->header()->resizeSection(taskcol_repository,  80);
	ui->taskTree->header()->resizeSection(taskcol_filename,    80);

	// create the top level entries in the tree
	{
		QStringList	list;
		list << "" << "" << "completed";
		QTreeWidgetItem	*item;
		item = new QTreeWidgetItem(list, QTreeWidgetItem::Type);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setTextAlignment(taskcol_exposure,    Qt::AlignRight);
		ui->taskTree->addTopLevelItem(item);
	}
	{
		QStringList	list;
		list << "" << "" << "cancelled";
		QTreeWidgetItem	*item;
		item = new QTreeWidgetItem(list, QTreeWidgetItem::Type);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setTextAlignment(taskcol_exposure,    Qt::AlignRight);
		ui->taskTree->addTopLevelItem(item);
	}
	{
		QStringList	list;
		list << "" << "" << "failed";
		QTreeWidgetItem	*item;
		item = new QTreeWidgetItem(list, QTreeWidgetItem::Type);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setTextAlignment(taskcol_exposure,    Qt::AlignRight);
		ui->taskTree->addTopLevelItem(item);
	}
	{
		QStringList	list;
		list << "" << "" << "executing";
		QTreeWidgetItem	*item;
		item = new QTreeWidgetItem(list, QTreeWidgetItem::Type);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setTextAlignment(taskcol_exposure,    Qt::AlignRight);
		ui->taskTree->addTopLevelItem(item);
	}
	{
		QStringList	list;
		list << "" << "" << "pending";
		QTreeWidgetItem	*item;
		item = new QTreeWidgetItem(list, QTreeWidgetItem::Type);
		item->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
		item->setTextAlignment(taskcol_exposure,    Qt::AlignRight);
		ui->taskTree->addTopLevelItem(item);
	}

	// connect buttons to slots
	connect(ui->infoButton, SIGNAL(clicked()),
		this, SLOT(infoClicked()));
	connect(ui->cancelButton, SIGNAL(clicked()),
		this, SLOT(cancelClicked()));
	connect(ui->imageButton, SIGNAL(clicked()),
		this, SLOT(imageClicked()));
	connect(ui->previewButton, SIGNAL(clicked()),
		this, SLOT(previewClicked()));
	connect(ui->downloadButton, SIGNAL(clicked()),
		this, SLOT(downloadClicked()));
	connect(ui->deleteButton, SIGNAL(clicked()),
		this, SLOT(deleteClicked()));
	connect(ui->resubmitButton, SIGNAL(clicked()),
		this, SLOT(resubmitClicked()));

	connect(ui->taskTree, SIGNAL(itemSelectionChanged()),
		this, SLOT(itemSelectionChanged()));

	connect(ui->taskTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
                this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));
	connect(ui->taskTree,
		SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		this,
		SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
}

/**
 * \brief Destroy the taskqueuemanagerwidget
 */
taskqueuemanagerwidget::~taskqueuemanagerwidget() {
	if (_taskmonitor) {
		Ice::Identity   identity = _taskmonitor->identity();
		snowstar::CommunicatorSingleton::remove(identity);
	}
	delete ui;
}

/**
 * \brief Add a task to a parent entry
 */
void	taskqueuemanagerwidget::addTask(QTreeWidgetItem *parent,
		const snowstar::TaskInfo& info,
		const snowstar::TaskParameters& parameters) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add task to '%s'",
		parent->text(1).toLatin1().data());
	astro::camera::Exposure	exposure = snowstar::convert(parameters.exp);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task type: %d", parameters.type);

	QStringList	list;

	// 0 taskid>
	list << QString::number(info.taskid);

	// 1 task type
	list << QString(tasktype2string(parameters.type).c_str());

	// 2 instrument
	list << QString(parameters.instrument.c_str());

	// 3 project
	list << QString(parameters.project.c_str());

	// 4 purpose
	if (parameters.type == snowstar::TaskEXPOSURE) {
		list << QString(astro::camera::Exposure::purpose2string(
			exposure.purpose()).c_str());
	} else {
		list << QString();
	}

	// 5 last state change
	time_t  when = snowstar::converttime(info.lastchange);
	struct tm       *tmp = localtime(&when);
	char    buffer[100];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	list << buffer;

	// 6 exposure time, 7 gain
	switch (parameters.type) {
	case snowstar::TaskEXPOSURE:
	case snowstar::TaskSLEEP:
		if (exposure.exposuretime() < 10) {
			list << QString(astro::stringprintf("%.3fs",
				exposure.exposuretime()).c_str());
			list << QString(astro::stringprintf("%.3f",
				exposure.gain()).c_str());
		} else {
			list << QString(astro::stringprintf("%.0fs",
				exposure.exposuretime()).c_str());
			list << QString();
		}
		break;
	case snowstar::TaskDITHER:
		list << QString(astro::stringprintf("%.1f\"",
			parameters.ccdtemperature).c_str());
			list << QString();
		break;
	case snowstar::TaskFOCUS:
		list << QString();
			list << QString();
		break;
	}

	if (parameters.type == snowstar::TaskEXPOSURE) {
		float	e = _totaltimes.find(info.state)->second;
		e += exposure.exposuretime();
		_totaltimes.find(info.state)->second = e;
	}

	// 8 filter
	if (parameters.type == snowstar::TaskEXPOSURE) {
		list << QString(parameters.filter.c_str());
	} else {
		list << QString();
	}

	// 9 frame
	if (parameters.type == snowstar::TaskEXPOSURE) {
		list << QString(astro::stringprintf("%dx%d",
				info.frame.size.width,
				info.frame.size.height).c_str());
	} else {
		list << QString();
	}

	// 10 binning
	if (parameters.type == snowstar::TaskEXPOSURE) {
		std::string	binning = exposure.mode().toString();
		list << QString(binning.substr(1, binning.size() - 2).c_str());
	} else {
		list << QString();
	}

	// 11 temperature
	if (parameters.type == snowstar::TaskEXPOSURE) {
		list << QString(astro::stringprintf("%.1f°C",
			parameters.ccdtemperature - astro::Temperature::zero).c_str());
	} else {
		list << QString();
	}

	// 12 repository
	if (parameters.type == snowstar::TaskEXPOSURE) {
		list << QString(parameters.repository.c_str());
	} else {
		list << QString();
	}

	// 13 repository database name
	if (parameters.type == snowstar::TaskEXPOSURE) {
		list << QString(parameters.repodb.c_str());
	} else {
		list << QString();
	}

	// 14 filename
	if (parameters.type == snowstar::TaskEXPOSURE) {
		list << QString(info.filename.c_str());
	} else {
		list << QString();
	}

	QTreeWidgetItem	*item = new QTreeWidgetItem(list,
		QTreeWidgetItem::Type);
	item->setTextAlignment(taskcol_id,          Qt::AlignRight);
	item->setTextAlignment(taskcol_type,        Qt::AlignLeft);
	item->setTextAlignment(taskcol_instrument,  Qt::AlignLeft);
	item->setTextAlignment(taskcol_project,     Qt::AlignLeft);
	item->setTextAlignment(taskcol_purpose,     Qt::AlignLeft);
	item->setTextAlignment(taskcol_lastchange,  Qt::AlignLeft);
	item->setTextAlignment(taskcol_exposure,    Qt::AlignRight);
	item->setTextAlignment(taskcol_gain,        Qt::AlignRight);
	item->setTextAlignment(taskcol_filter,      Qt::AlignLeft);
	item->setTextAlignment(taskcol_frame,       Qt::AlignLeft);
	item->setTextAlignment(taskcol_binning,     Qt::AlignLeft);
	item->setTextAlignment(taskcol_temperature, Qt::AlignRight);
	item->setTextAlignment(taskcol_repository,  Qt::AlignLeft);
	item->setTextAlignment(taskcol_database,    Qt::AlignLeft);
	item->setTextAlignment(taskcol_filename,    Qt::AlignLeft);

	// now add the new entry to the parent
	parent->addChild(item);
}

/**
 * \brief add a task based in the id
 */
void	taskqueuemanagerwidget::addTask(int taskid) {
	if (!_tasks) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add task %d", taskid);
	try {
		snowstar::TaskInfo	info = _tasks->info(taskid);
		snowstar::TaskParameters	parameters = _tasks->parameters(taskid);
		QTreeWidgetItem	*parent = this->parent(info.state);
	
		addTask(parent, info, parameters);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get task %d: %s",
			taskid, x.what());
	}
}

/**
 * \brief add all tasks of a given state
 */
void	taskqueuemanagerwidget::addTasks(QTreeWidgetItem *parent,
		snowstar::TaskState state) {
	if (!_tasks) {
		return;
	}
	snowstar::taskidsequence	s;
	try {
		s = _tasks->tasklist(state);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get task list: %s",
			x.what());
	}
	for (auto i = s.begin(); i != s.end(); i++) {
		try {
			snowstar::TaskInfo	info = _tasks->info(*i);
			snowstar::TaskParameters	parameters
				= _tasks->parameters(*i);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "task %d: repodb: %s",
				info.taskid,
				parameters.repodb.c_str());
			addTask(parent, info, parameters);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot get tasks for state %d", state);
		}
	}
	setHeaders();
}

/**
 * \brief Update a header for a given state based
 */
void	taskqueuemanagerwidget::setHeader(snowstar::TaskState state) {
	std::string	tag;
	QTreeWidgetItem	*top;
	int	count;
	float	exposuretime = _totaltimes.find(state)->second;
	switch (state) {
	case snowstar::TskCOMPLETE:
		top = ui->taskTree->topLevelItem(0);
		tag = "completed";
		break;
        case snowstar::TskCANCELLED:
		top = ui->taskTree->topLevelItem(1);
		tag = "cancelled";
		break;
        case snowstar::TskFAILED:
		top = ui->taskTree->topLevelItem(2);
		tag = "failed";
		break;
        case snowstar::TskEXECUTING:
		top = ui->taskTree->topLevelItem(3);
		tag = "executing";
		break;
        case snowstar::TskPENDING:
		top = ui->taskTree->topLevelItem(4);
		tag = "pending";
		break;
        case snowstar::TskDELETED:
		throw std::logic_error("cannot set header for deleted");
		break;
	}
	count = top->childCount();
	top->setText(2, QString(astro::stringprintf("%s (%d)",
			tag.c_str(), count).c_str()));
	top->setTextAlignment(5, Qt::AlignRight);
	if (count > 0) {
		top->setText(taskcol_exposure,
			QString(astro::stringprintf("%.0fs",
				exposuretime).c_str()));
	} else {
		top->setText(taskcol_exposure, QString());
	}
}

/**
 * \brief Update all headers to reflect the correct number of entries
 */
void	taskqueuemanagerwidget::setHeaders() {
	setHeader(snowstar::TskCOMPLETE);
	setHeader(snowstar::TskCANCELLED);
	setHeader(snowstar::TskFAILED);
	setHeader(snowstar::TskEXECUTING);
	setHeader(snowstar::TskPENDING);
}

/**
 * \brief add all tasks found in the repository
 */
void	taskqueuemanagerwidget::addTasks() {
	if (!_tasks) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"no tasks proxy, cannot add tasks");
		return;
	}

	addTasks(ui->taskTree->topLevelItem(0), snowstar::TskCOMPLETE);
	addTasks(ui->taskTree->topLevelItem(1), snowstar::TskCANCELLED);
	addTasks(ui->taskTree->topLevelItem(2), snowstar::TskFAILED);
	addTasks(ui->taskTree->topLevelItem(3), snowstar::TskEXECUTING);
	addTasks(ui->taskTree->topLevelItem(4), snowstar::TskPENDING);
}

/**
 * \brief set the service objecth
 *
 * This method also initializes the proxies and adds all tasks to to
 * task tree.
 */
void	taskqueuemanagerwidget::setServiceObject(
		astro::discover::ServiceObject serviceobject) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set service object");
	// get the Tasks proxy
	Ice::CommunicatorPtr    ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx  base = ic->stringToProxy(
		serviceobject.connect("Tasks"));
	_tasks = snowstar::TaskQueuePrx::checkedCast(base);
	if (!_tasks) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get a taskqueue");
	}

	_taskmonitor = new TaskMonitorController(NULL);
	_taskmonitorptr = Ice::ObjectPtr(_taskmonitor);
	_taskmonitor->setTasks(_tasks, _taskmonitorptr);

	// connect the task monitor to this widget
	connect(_taskmonitor, SIGNAL(taskUpdate(snowstar::TaskMonitorInfo)),
		this, SLOT(taskUpdate(snowstar::TaskMonitorInfo)));

	// get the repositories proxy
	base = ic->stringToProxy(serviceobject.connect("Repositories"));
	_repositories = snowstar::RepositoriesPrx::checkedCast(base);
	if (!_repositories) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get the repositories");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service setup complete");

	// connect to the images proxy
	base = ic->stringToProxy(serviceobject.connect("Images"));
	_images = snowstar::ImagesPrx::checkedCast(base);
	if (!_images) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get the images");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "service setup complete");

	// add the tasks
	addTasks();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set service object complete");
}

void	taskqueuemanagerwidget::infoClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "infoClicked()");
	QList<QTreeWidgetItem*>	selected = ui->taskTree->selectedItems();
	if (selected.size() != 1) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"ignoring infoClicked(): more than one item selected");
		return;
	}
	showInfo(*selected.begin());
	return;
}

void	taskqueuemanagerwidget::cancelClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cancelClicked()");
	std::list<int>	taskids = selectedids();
	for (auto idptr = taskids.begin(); idptr != taskids.end(); idptr++) {
		int	taskid = *idptr;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cancel task %d", taskid);
		try {
			_tasks->cancel(taskid);
		} catch (const snowstar::BadState& badstate) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"taskid %d: cannot cancel, bad state",
				badstate.what());
		} catch (const snowstar::NotFound& notfound) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"taskid %d: cannot cancel, not found, %s",
				notfound.what());
		} catch (const std::exception& x) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot cancel: %s",
				x.what());
		}
	}
}

void	taskqueuemanagerwidget::showImage(snowstar::ImageEncoding encoding) {
	QList<QTreeWidgetItem*>	selected = ui->taskTree->selectedItems();
	if (selected.size() != 1) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "more than one object selected");
		return;
	}
	int	taskid = std::stoi((*selected.begin())->text(0).toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task id = %d", taskid);
	try {
		// return if not an exposure task
		snowstar::TaskParameters	parameters
			= _tasks->parameters(taskid);
		if (parameters.type != snowstar::TaskEXPOSURE) {
			return;
		}
		// return if not complete
		snowstar::TaskInfo	info = _tasks->info(taskid);
		if (info.state != snowstar::TskCOMPLETE) {
			return;
		}
		// now get the image
		ImagePtr	imageptr;
		if (parameters.repository.size() > 0) {
			int	imageid = std::stoi(info.filename);
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"getting image %d from repository %s", imageid,
				parameters.repository.c_str());
			snowstar::RepositoryPrx	repository
				= _repositories->get(parameters.repository);
			if (!repository->has(imageid)) {
				return;
			}
			imageptr = snowstar::convertimage(
				repository->getImage(imageid, encoding));
		} else {
			// XXX encoding...
			debug(LOG_DEBUG, DEBUG_LOG, 0, "get image %s from dir",
				info.filename.c_str());
			snowstar::ImagePrx	imageprx
				= _images->getImage(info.filename);
			snowstar::ImageBuffer	buffer
				= imageprx->file(encoding);
			imageptr = snowstar::convertimage(buffer);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "emitting imageReceived()");
		emit imageReceived(imageptr);
	} catch (const std::exception& x) {
		// XXX show error message
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error: %s", x.what());
	}
}

/**
 * \brief slot to display the image
 */
void	taskqueuemanagerwidget::imageClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imageClicked()");
	showImage(snowstar::ImageEncodingFITS);
}

void	taskqueuemanagerwidget::previewClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "previewClicked()");
	showImage(snowstar::ImageEncodingJPEG);
}

/**
 * \brief Slot to handle clicks on the download button
 */
void	taskqueuemanagerwidget::downloadClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "downloadClicked()");
}

/**
 * \brief Slot to handle clicks on the delete button
 */
void	taskqueuemanagerwidget::deleteClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deleteClicked()");
	QList<QTreeWidgetItem*>	todelete = ui->taskTree->selectedItems();
	std::list<int>	taskids = selectedids();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d items to delete", todelete.size());
	for (auto j = taskids.begin(); j != taskids.end(); j++) {
		try {
			_tasks->remove(*j);
			deleteTask(*j);
		} catch (const std::exception& x) {
			// XXX pop message
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot delete %d: %s",
				*j, x.what());
		}
	}
	setHeaders();
}

/**
 * \brief Slot to handle resubmission
 */
void	taskqueuemanagerwidget::resubmitClicked() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "resubmitClicked()");
	QList<QTreeWidgetItem*>	todelete = ui->taskTree->selectedItems();
	std::list<int>	taskids = selectedids();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d items to delete", todelete.size());
	for (auto j = taskids.begin(); j != taskids.end(); j++) {
		try {
			_tasks->resubmit(*j);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot resubmit %d: %s",
				*j, x.what());
		}
	}
}

/**
 * \brief Reflect changed information in the task list entries
 */
void	taskqueuemanagerwidget::updateInfo(QTreeWidgetItem *item,
		const snowstar::TaskInfo& info) {
	switch (info.state) {
	case snowstar::TskPENDING:
	case snowstar::TskEXECUTING:
		item->setText(taskcol_filename, QString(""));
		break;
	case snowstar::TskFAILED:
	case snowstar::TskCANCELLED:
		item->setText(taskcol_filename, QString(info.cause.c_str()));
		break;
	case snowstar::TskCOMPLETE:
		item->setText(taskcol_filename, QString(info.filename.c_str()));
		break;
	case snowstar::TskDELETED:
		return;
	}
	if ((info.frame.size.width != 0) && (info.frame.size.height != 0)) {
		item->setText(taskcol_frame,
			QString(astro::stringprintf("%dx%d",
			info.frame.size.width,
			info.frame.size.height).c_str()));
	}

	// 4 last state change
	time_t  when = snowstar::converttime(info.lastchange);
	struct tm       *tmp = localtime(&when);
	char    buffer[100];
	strftime(buffer, sizeof(buffer), "%F %T", tmp);
	item->setText(taskcol_lastchange, QString(buffer));
}

/**
 * \brief update a task
 */
void	taskqueuemanagerwidget::taskUpdate(snowstar::TaskMonitorInfo info) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "task udpate for %d", info.taskid);

	// if the state is pending, then this is new entry and we have to
	// add that entry to the pending section
	if (info.newstate == snowstar::TskPENDING) {
		addTask(info.taskid);
		setHeaders();
		return;
	}

	// get the task information
	snowstar::TaskInfo	tinfo;
	bool	hasinfo = false;
	try {
		if (_tasks) {
			tinfo = _tasks->info(info.taskid);
			hasinfo = true;
		}
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get task info %d",
			info.taskid);
	}

	// for all other cases, we don't have to create new entries, but
	// only move them around.
	int	tasksection = -1;
	for (int section = 0; section < 5; section++) {
		QString id = QString::number(info.taskid);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"checking section %d, task id %s", section,
			id.toLatin1().data());
		QTreeWidgetItem	*top = ui->taskTree->topLevelItem(section);

		// go through all the children of the top level node in the
		// hope of finding an item with the same id
		for (int i = 0; i < top->childCount(); i++) {
			QTreeWidgetItem	*child = top->child(i);
			if (child->text(0) == id) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "found item");
				tasksection = section;
				child = top->takeChild(i);
				if (info.newstate != snowstar::TskDELETED) {
					if (hasinfo) {
						updateInfo(child, tinfo);
					}
					this->parent(info.newstate)->addChild(child);
				}
				goto summarize;
			}
		}
	}
	return;

summarize:
	if (_tasks->parameters(info.taskid).type == snowstar::TaskEXPOSURE) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "skipping exposure task %d",
			info.taskid);
		// get the task parameters
		float	exposuretime = _tasks->parameters(info.taskid).exp.exposuretime;

		// remove the time from the section where we found it
		snowstar::TaskState	state;
		switch (tasksection) {
		case 0:	state = snowstar::TskCOMPLETE;
			break;
		case 1:	state = snowstar::TskFAILED;
			break;
		case 2:	state = snowstar::TskCANCELLED;
			break;
		case 3:	state = snowstar::TskEXECUTING;
			break;
		case 4:	state = snowstar::TskPENDING;
			break;
		}
		_totaltimes.find(state)->second = 
			_totaltimes.find(state)->second - exposuretime;

		// add the time to the section to which we moved it
		if (info.newstate != snowstar::TskDELETED) {
			_totaltimes.find(tinfo.state)->second = 
				_totaltimes.find(tinfo.state)->second
					+ exposuretime;
		}
	}

	// display the headers
	setHeaders();
}

/**
 * \brief Find the top level item for this state
 */
QTreeWidgetItem	*taskqueuemanagerwidget::parent(snowstar::TaskState state) {
	switch (state) {
	case snowstar::TskCOMPLETE:
		return ui->taskTree->topLevelItem(0);
	case snowstar::TskCANCELLED:
		return ui->taskTree->topLevelItem(1);
	case snowstar::TskFAILED:
		return ui->taskTree->topLevelItem(2);
	case snowstar::TskEXECUTING:
		return ui->taskTree->topLevelItem(3);
	case snowstar::TskPENDING:
		return ui->taskTree->topLevelItem(4);
	case snowstar::TskDELETED:
		return NULL;
	}
	throw std::range_error("bad state value");
}

/**
 * \brief delete a task tree entry based on the task id
 */
void	taskqueuemanagerwidget::deleteTask(int taskid) {
	// remove the task from the list
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete with task id = %d", taskid);
	for (int section = 0; section < 5; section++) {
		QString	id = QString::number(taskid);
		QTreeWidgetItem	*top = ui->taskTree->topLevelItem(section);
		for (int i = 0; i < top->childCount(); i++) {
			QTreeWidgetItem *child = top->child(i);
			if (child->text(0) == id) {
				child = top->takeChild(i);
				delete child;
				goto summarize;
			}
		}
	}
	return;

summarize:
	// get the task information from the database to update the total time
	try {
		snowstar::TaskParameters	tp = _tasks->parameters(taskid);
		if (tp.type == snowstar::TaskEXPOSURE) {
			snowstar::TaskInfo	ti = _tasks->info(taskid);
			float	f = _totaltimes.find(ti.state)->second;
			f -= _tasks->parameters(taskid).exp.exposuretime;
			_totaltimes.find(ti.state)->second = f;
		}
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not get task info %d: %s",
			taskid, x.what());
	}
}

/**
 * \brief Find out which buttons to enable for the current selection
 */
void	taskqueuemanagerwidget::itemSelectionChanged() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "itemSelectionChanged()");
	QList<QTreeWidgetItem*>	selected = ui->taskTree->selectedItems();
	if (1 == selected.count()) {
		ui->infoButton->setEnabled(true);
		bool	imageenabled = (ui->taskTree->topLevelItem(0)
					== (*selected.begin())->parent());
		ui->imageButton->setEnabled(imageenabled);
		ui->previewButton->setEnabled(imageenabled);
	} else {
		ui->infoButton->setEnabled(false);
		ui->imageButton->setEnabled(false);
		ui->previewButton->setEnabled(false);
	}
	if (selected.count() > 0) {
		ui->cancelButton->setEnabled(true);
		ui->deleteButton->setEnabled(true);
		ui->resubmitButton->setEnabled(true);
		ui->downloadButton->setEnabled(true);
	} else {
		ui->cancelButton->setEnabled(false);
		ui->deleteButton->setEnabled(false);
		ui->resubmitButton->setEnabled(false);
		ui->downloadButton->setEnabled(false);
	}
}

/**
 * \brief Display information about a given task id
 */
void	taskqueuemanagerwidget::showInfo(int taskid) {
	// if the task info widget already exists, just update the information
	if (_taskinfowidget) {
		_taskinfowidget->updateTask(taskid);
		return;
	}
	_taskinfowidget = new taskinfowidget(this);
	connect(_taskinfowidget, SIGNAL(completed()),
		this, SLOT(forgetInfoWidget()));
	_taskinfowidget->setProxies(_tasks, _images, _repositories);
	_taskinfowidget->updateTask(taskid);
	_taskinfowidget->show();
}

/**
 * \brief Display information about a given entry in the task list
 */
void	taskqueuemanagerwidget::showInfo(QTreeWidgetItem *item) {
	std::string	idstring(item->text(0).toLatin1().data());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "item id '%s' double clicked",
		idstring.c_str());
	if (idstring.size() == 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring infoClicked(): no id");
		return;
	}
	int	taskid = std::stoi(idstring);
	showInfo(taskid);
}

/**
 * \brief Slot to handle double click action
 */
void	taskqueuemanagerwidget::itemDoubleClicked(QTreeWidgetItem *item, int) {
	showInfo(item);
}

/**
 * \brief Handle changed current item, if the info widget is already active
 */
void	taskqueuemanagerwidget::currentItemChanged(QTreeWidgetItem *item,
		QTreeWidgetItem*) {
	if (NULL == _taskinfowidget) {
		return;
	}
	showInfo(item);
}

/**
 * \brief Slot to handle closing of the info widget
 */
void	taskqueuemanagerwidget::forgetInfoWidget() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "forget the info widget");
	_taskinfowidget = NULL;
}

/**
 * \brief retrieve the currently selected ids
 */
std::list<int>	taskqueuemanagerwidget::selectedids() {
	QList<QTreeWidgetItem*>	selected = ui->taskTree->selectedItems();
	std::list<int>	taskids;
	for (auto i = selected.begin(); i != selected.end(); i++) {
		taskids.push_back(std::stoi((*i)->text(0).toLatin1().data()));
	}
	return taskids;
}

} // namespace snowgui
