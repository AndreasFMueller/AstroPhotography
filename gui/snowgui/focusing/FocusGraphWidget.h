/*
 * FocusGraphWidget.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _FocusGraphWidget_h
#define _FocusGraphWidget_h

#include <QWidget>
#include <focusing.h>
#include <list>

namespace snowgui {

class FocusGraphWidget : public QWidget {
	Q_OBJECT
	std::list<snowstar::FocusPoint>	_points;

	snowstar::FocusState	_state;

public:
	explicit FocusGraphWidget(QWidget *parent = NULL);
	virtual ~FocusGraphWidget();

	void	paintEvent(QPaintEvent *);

public slots:
	void	receivePoint(snowstar::FocusPoint);
	void	receiveState(snowstar::FocusState);
};

} // namespace snowgui

#endif /* _FocusGraphWidget_h */
