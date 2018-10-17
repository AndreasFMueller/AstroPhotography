/*
 * lifeview.h -- lifeview application main window header
 */
#ifndef SNOWGUI_LIVEVIEW_H
#define SNOWGUI_LIVEVIEW_H

#include <QMainWindow>

namespace snowgui {

namespace Ui {
	class LiveView;
}

class LiveView : public QMainWindow {
	Q_OBJECT

	std::list<std::string>	_ccdNames;
	std::list<std::string>	_focuserNames;

	QMenu	*_ccdMenu;
	std::list<QAction*>	_ccdActions;

public:
	explicit LiveView(QWidget *parent = 0);
	~LiveView();

private:
	Ui::LiveView *ui;

public slots:
	void	openCamera();
};

} // namespace snowgui

#endif // SNOWGUI_LIVEVIEW_H
