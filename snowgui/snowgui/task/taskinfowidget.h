/*
 * taskinfowidget.h -- dialog widget to display information about a task
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TASKINFOWIDGET_H
#define SNOWGUI_TASKINFOWIDGET_H

#include <QDialog>
#include <tasks.h>
#include <image.h>
#include <repository.h>
#include <TaskMonitorController.h>

namespace snowgui {

namespace Ui {
	class taskinfowidget;
}

class taskinfowidget : public QDialog {
	Q_OBJECT

	snowstar::TaskQueuePrx	_tasks;
	snowstar::ImagesPrx	_images;
	snowstar::RepositoriesPrx	_repositories;
	TaskMonitorController	*_taskmonitor;
	Ice::ObjectPtr	_taskmonitorptr;
	int	_taskid;

public:
	explicit taskinfowidget(QWidget *parent = 0);
	~taskinfowidget();

	void	setProxies(snowstar::TaskQueuePrx, snowstar::ImagesPrx,
			snowstar::RepositoriesPrx);
	virtual void	closeEvent(QCloseEvent *);

signals:
	void	completed();

public slots:
	void	updateTask(int taskid);
	void	refreshClicked();
	void	imageClicked();
	void	closeClicked();
	void	taskUpdate(snowstar::TaskMonitorInfo);

private:
	Ui::taskinfowidget *ui;
};

} // namespace snowgui

#endif // SNOWGUI_TASKINFOWIDGET_H
