/*
 * focusingprogresswidget.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_FOCUSINGPROGRESSWIDGET_H
#define SNOWGUI_FOCUSINGPROGRESSWIDGET_H

#include <QWidget>
#include <focusing.h>

namespace snowgui {

namespace Ui {
	class FocusingProgressWidget;
}

class FocusingProgressWidget : public QWidget {
	Q_OBJECT

	bool	_focused;

public:
	explicit FocusingProgressWidget(QWidget *parent = 0);
	~FocusingProgressWidget();

private:
	Ui::FocusingProgressWidget *ui;

public slots:
	void	receivePoint(snowstar::FocusPoint);
	void	receiveState(snowstar::FocusState);
};

} // namespace snowgui
#endif // SNOWGUI_FOCUSINGPROGRESSWIDGET_H
