/*
 * TasksIndicator.h -- indicator to show the current task queue status
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _TasksIndicator_h
#define _TasksIndicator_h

#include <QWidget>
#include <tasks.h>

namespace snowgui {

class TasksIndicator : public QWidget {
	Q_OBJECT
private:
	snowstar::QueueState	_state;
public:
	explicit TasksIndicator(QWidget *parent = NULL);
	~TasksIndicator();
	void	draw();
	void	paintEvent(QPaintEvent *event);
public slots:
	void	update(snowstar::QueueState);
};

} // namespace snowgui

#endif /* _TasksIndicator_h */
