/*
 * LogSpingBox.h -- logarithmic spin box used for exposure times
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _logspingbox_h
#define _logspingbox_h

#include <QDoubleSpinBox>

class LogSpinBox : public QDoubleSpinBox {
	Q_OBJECT

	int	exponent;
	double	step;
private:
	double	upstep();
	double	downstep();
public:
	LogSpinBox(QWidget *parent = NULL);
	virtual void stepBy(int steps);
};

#endif /* _logspinbox_h */
