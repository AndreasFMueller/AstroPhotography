/*
 * taskqueuemanagerwidget.h -- manage a task queue
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_TASKQUEUEMANAGERWIDGET_H
#define SNOWGUI_TASKQUEUEMANAGERWIDGET_H

#include <QWidget>

namespace snowgui {

namespace Ui {
	class taskqueuemanagerwidget;
}

class taskqueuemanagerwidget : public QWidget {
	Q_OBJECT

public:
	explicit taskqueuemanagerwidget(QWidget *parent = 0);
	~taskqueuemanagerwidget();

private:
	Ui::taskqueuemanagerwidget *ui;
};

} // namespace snowgui

#endif // SNOWGUI_TASKQUEUEMANAGERWIDGET_H
