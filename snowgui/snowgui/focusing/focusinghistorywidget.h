/*
 * focusinghistorywidget.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef FOCUSINGHISTORYWIDGET_H
#define FOCUSINGHISTORYWIDGET_H

#include <QWidget>
#include <AstroImage.h>

namespace Ui {
	class focusinghistorywidget;
}

namespace snowgui {

class focusinghistorywidget : public QWidget {
	Q_OBJECT

public:
	explicit focusinghistorywidget(QWidget *parent = 0);
	~focusinghistorywidget();

	void	add(astro::image::ImagePtr, unsigned short);

signals:
	void	positionSelected(int);

private:
	Ui::focusinghistorywidget *ui;

public slots:
	void	clear();
	void	byPosition(bool);
	void	bySequence(bool);
	void	didSelectPosition(int);
};

} // namespace snowgui

#endif // FOCUSINGHISTORYWIDGET_H
