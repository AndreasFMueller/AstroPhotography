//
// RotateDial.h
//
// (c) 2019 Prof Dr Andreas Müller, Hochschule Rapeprswi
//

#ifndef _RotateDial_h
#define _RotateDial_h

#include <QDial>

namespace snowgui {

class RotateDial : public QDial {
	Q_OBJECT
public:
	explicit RotateDial(QWidget *parent = NULL);
	void	paintEvent(QPaintEvent *event);
};

} // namespace snowgui

#endif /* _RotateDial_h */
