/*
 * FocusButton.h --
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusButton_h
#define _FocusButton_h

#include <QPushButton>

namespace snowgui {

class FocusButton : public QPushButton {
	Q_OBJECT

	double	_f;
	void	draw();
public:
	explicit FocusButton(QWidget *parent = NULL);
	~FocusButton();

public slots:
	void	paintEvent(QPaintEvent *);
	void	update(double f);
};

} // namespace snowgui

#endif /* _FocusButton_h */
