/*
 * GuiderButton.h -- Button for the guider port control widget
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderButton_h
#define _GuiderButton_h

#include <QWidget>

namespace snowgui {

class GuiderButton : public QWidget {
	Q_OBJECT

	bool	_northPressed;
        bool	_southPressed;
        bool	_eastPressed;
        bool	_westPressed;

	double	angle(QPoint p);
	void	checkPressed(QPoint p);
public:
	explicit GuiderButton(QWidget *parent = NULL);
	virtual ~GuiderButton();

	void	draw();
signals:
	void	northClicked();
	void	southClicked();
	void	eastClicked();
	void	westClicked();

public slots:
	void	paintEvent(QPaintEvent *event);
	void	mousePressEvent(QMouseEvent *e);
	void	mouseMoveEvent(QMouseEvent *e);
	void	mouseReleaseEvent(QMouseEvent *e);
};

} // namespace snowgui

#endif /* _GuiderButton_h */
