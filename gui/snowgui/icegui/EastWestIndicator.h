/*
 * EastWestIndicator.h -- indicator to show whether the telescope is on the
 *                        east or west side of the mount
 *
 * (c) 2021 Prof Dr Andreas Müller, OST Ostschweizer Fachhochschule
 */
#ifndef _EastWestIndicator_h
#define _EastWestIndicator_h

#include <QWidget>
#include <tasks.h>

namespace snowgui {

class EastWestIndicator : public QWidget {
	Q_OBJECT
private:
	bool	_north;
public:
	bool	north() const { return _north; }
	void	north(bool n) { _north = n; }
private:
	bool	_east;
	void	draw();
public:
	explicit EastWestIndicator(QWidget *parent = NULL);
	~EastWestIndicator();
	void	paintEvent(QPaintEvent *event);
public slots:
	void	update(bool east);
};

} // namespace snowgui

#endif /* _EastWestIndicator_h */
