/*
 * LocalTimeWidget.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _LocalTimeWidget_h
#define _LocalTimeWidget_h

#include <QLabel>
#include <QTimer>

namespace snowgui {

class LocalTimeWidget : public QLabel {
	Q_OBJECT

	QTimer	_statusTimer;

public:
	explicit LocalTimeWidget(QWidget *parent = NULL);
	~LocalTimeWidget();

public slots:
	void	statusUpdate();
};

} // namespace snowgui

#endif /* _LocalTimeWidget_h */
