/*
 * CoolerActive.h -- widget to display the cooler's activity
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _CoolerActive_h
#define _CoolerActive_h

#include <QPushButton>
#include <QEvent>

namespace snowgui {

/**
 * \brief cooler activity display widget
 *
 * This widget is essentially a pushbutton, but with a different type of
 * display
 */
class CoolerActive : public QPushButton {
	Q_OBJECT

	bool	_active;
	double	_value;
public:
	explicit CoolerActive(QWidget *parent = NULL);
	virtual ~CoolerActive();

	bool	active() const { return _active; }
	bool	value() const { return _value; }
	void	setActive(bool b);
	void	setValue(double v);
	void	paintEvent(QPaintEvent *event);
	void	draw();

signals:
	void	toggled(bool);

public slots:
	void    update(float actualtemp, float settemp, bool active);
	void	update();
	void	buttonClicked();
};

} // namespace snowgui

#endif /* _CoolerActive_h */
