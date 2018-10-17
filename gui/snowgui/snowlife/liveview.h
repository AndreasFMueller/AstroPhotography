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

	QMenu	*_focuserMenu;
	std::list<QAction*>	_focuserActions;

public:
	explicit LiveView(QWidget *parent = 0);
	~LiveView();

private:
	Ui::LiveView *ui;

public slots:
	void	openCamera();
	void	openFocuser();
	void	addCamera(std::string);
	void	addFocuser(std::string);
};

} // namespace snowgui

#endif // SNOWGUI_LIVEVIEW_H
