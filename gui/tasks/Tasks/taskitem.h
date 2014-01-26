/*
 * taskitem.h -- Item widget for the task list
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _taskitem_h
#define _taskitem_h

#include <QWidget>
#include <tasks.hh>
#include <QPushButton>

class TaskItem : public QWidget {
	Q_OBJECT

	QPushButton	*button;
	Astro::TaskInfo	info;
	Astro::TaskParameters	parameters;	
public:
	explicit TaskItem(const Astro::TaskInfo& _info,
			const Astro::TaskParameters& _parameters,
			QWidget *parent = 0);
	virtual ~TaskItem();
	void	paintEvent(QPaintEvent *event);
	void	updateInfo(const Astro::TaskInfo& newinfo);
	int	id() const;
private:	
	void	draw();
signals:
	void	buttonSignal(int taskid);

public slots:
	void	handleButton();
};

#endif /* _taskitem_h */
