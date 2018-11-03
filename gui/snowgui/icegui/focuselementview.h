/*
 * focuselementview.h -- show the two images of a focus element side by side
 *
 * (c) 2018 Prof Dr Andreas Müller
 */
#ifndef SNOWGUI_FOCUSELEMENTVIEW_H
#define SNOWGUI_FOCUSELEMENTVIEW_H

#include <QWidget>
#include <focusing.h>

namespace snowgui {

namespace Ui {
	class FocusElementView;
}

class FocusElementView : public QWidget {
	Q_OBJECT

	snowstar::FocusElement	_element;

public:
	explicit FocusElementView(QWidget *parent = 0);
	~FocusElementView();

private:
	Ui::FocusElementView *ui;

public slots:
	void	setFocusElement(snowstar::FocusElement);

	void	sliderChanged(int);
};

} // namespace snowgui
#endif // SNOWGUI_FOCUSELEMENTVIEW_H
