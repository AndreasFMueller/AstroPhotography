/*
 * localconfigurationwidget.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_LOCALCONFIGURATIONWIDGET_H
#define SNOWGUI_LOCALCONFIGURATIONWIDGET_H

#include <QWidget>

namespace snowgui {

namespace Ui {
	class localconfigurationwidget;
}

class localconfigurationwidget : public QWidget {
	Q_OBJECT

public:
	explicit localconfigurationwidget(QWidget *parent = nullptr);
	~localconfigurationwidget();

private:
	Ui::localconfigurationwidget *ui;
};

} // namespace snowgui

#endif // SNOWGUI_LOCALCONFIGURATIONWIDGET_H
