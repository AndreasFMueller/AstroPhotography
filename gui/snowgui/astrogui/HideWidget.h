/*
 * HideWidget.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _HideWidget_h
#define _HideWidget_h

#include <QWidget>
#include <QTimer>

namespace snowgui {

class HideWidget : public QWidget {
	Q_OBJECT

	QString	_text;
	QTimer	_timer;
	bool	_hide;
public:
	HideWidget(QString text, QWidget *parent = NULL);
	virtual ~HideWidget();

	QString	text() { return _text; }
private:
	void	draw();

protected:
	void	paintEvent(QPaintEvent *event);

public slots:
	void	setText(QString);
	void	timeout();
};

} // namespace snowgui

#endif /* _HideWidget_h */
