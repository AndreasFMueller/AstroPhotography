/*
 * focuselementstack.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochchule Rapperswil
 */
#ifndef SNOWGUI_FOCUSELEMENTSTACK_H
#define SNOWGUI_FOCUSELEMENTSTACK_H

#include <QWidget>
#include <QStackedWidget>
#include <focusing.h>

namespace snowgui {

namespace Ui {
	class FocusElementStack;
}

class FocusElementStack : public QStackedWidget {
	Q_OBJECT

	bool	_restart;

public:
	explicit FocusElementStack(QWidget *parent = 0);
	~FocusElementStack();

public slots:
	void	receiveFocusElement(snowstar::FocusElement);
	void	receiveState(snowstar::FocusState);
};

} // namespace snowgui

#endif // SNOWGUI_FOCUSELEMENTSTACK_H
