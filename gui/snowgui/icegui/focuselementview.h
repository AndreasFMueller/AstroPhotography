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

	bool	_show_rawimage;
	bool	_show_evaluatedimage;

public:
	explicit FocusElementView(QWidget *parent = 0);
	~FocusElementView();

	bool	show_rawimage() const { return _show_rawimage; }
	void	show_rawimage(bool r);
	bool	show_evaluatedimage() const { return _show_evaluatedimage; }
	void	show_evaluatedimage(bool p);

private:
	Ui::FocusElementView *ui;

public slots:
	void	setFocusElement(snowstar::FocusElement);
	void	sliderChanged(int);
	void	showContextMenu(const QPoint& point);
	void	setShowRawimage(bool);
	void	setShowEvaluatedimage(bool);
	void	toggleShowRawimage();
	void	toggleShowEvaluatedimage();
};

} // namespace snowgui
#endif // SNOWGUI_FOCUSELEMENTVIEW_H
