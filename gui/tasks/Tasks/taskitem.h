/*
 * taskitem.h -- Item widget for the task list
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _taskitem_h
#define _taskitem_h

#include <QWidget>
#include <tasks.hh>

class TaskItem : public QWidget {

	Astro::TaskInfo	info;
public:
	int	id() const { return info.taskid; }
private:
	Astro::TaskParameters	parameters;	
	void	draw();
public:
	TaskItem(const Astro::TaskInfo& _info,
		const Astro::TaskParameters& _parameters,
		QWidget *parent = 0);
	virtual ~TaskItem();
	void	paintEvent(QPaintEvent *event);
	void	updateInfo(const Astro::TaskInfo& newinfo) {
		info = newinfo;
	}
};

#endif /* _taskitem_h */
