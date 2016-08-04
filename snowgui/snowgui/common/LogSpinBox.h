/*
 * LogSpingBox.h -- logarithmic spin box used for exposure times
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _LogSpinBox_h
#define _LogSpinBox_h

#include <QDoubleSpinBox>

namespace snowgui {

class LogSpinBox : public QDoubleSpinBox {
	Q_OBJECT

private:
	double	upstep();
	double	downstep();
public:
	LogSpinBox(QWidget *parent = NULL);
	virtual void stepBy(int steps);
};

} // namespace snowgui

#endif /* _LogSpinBox_h */
