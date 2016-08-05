/*
 * calibrationwidget.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef CALIBRATIONWIDGET_H
#define CALIBRATIONWIDGET_H

#include <QWidget>

namespace Ui {
	class calibrationwidget;
}

namespace snowgui {

class calibrationwidget : public QWidget
{
	Q_OBJECT

public:
	explicit calibrationwidget(QWidget *parent = 0);
	~calibrationwidget();

private:
	Ui::calibrationwidget *ui;
};

} // namespace snowgui

#endif // CALIBRATIONWIDGET_H
