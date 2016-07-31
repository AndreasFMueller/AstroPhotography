/*
 * SelectableImage.h -- class derived from QLabel with 
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _SelectableImage_h
#define _SelectableImage_h

#include <QLabel>
#include <QRubberBand>

namespace snowgui {

class SelectableImage : public QLabel {
	Q_OBJECT
	QRubberBand	*rubberband;
	QPoint	origin;
public:
	SelectableImage(QWidget *parent = NULL);
signals:
	void	selectionCompleted(QRect*);
protected:
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
};

} // namespace snowgui

#endif /* _SelectableImage_h */
