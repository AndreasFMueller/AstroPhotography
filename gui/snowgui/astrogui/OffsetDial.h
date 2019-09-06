//
// OffsetDial.h
//
// (c) 2019 Prof Dr Andreas Müller, Hochschule Rapeprswi
//

#ifndef _OffsetDial_h
#define _OffsetDial_h

#include <QDial>

namespace snowgui {

class OffsetDial : public QDial {
	Q_OBJECT
public:
	explicit OffsetDial(QWidget *parent = NULL);
	void	paintEvent(QPaintEvent *event);
};

} // namespace snowgui

#endif /* _OffsetDial_h */
