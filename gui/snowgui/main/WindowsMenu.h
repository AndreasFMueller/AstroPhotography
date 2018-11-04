/*
 * WindowsMenu.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _WindowsMenu_h
#define _WindowsMenu_h

#include <QObject>
#include <QMenu>
#include <QAction>
#include <QWindow>

namespace snowgui {

class WindowsMenu : public QObject {
	Q_OBJECT

	QMenu	*_menu;
	std::vector<QWidget *>	_widgets;
	std::vector<QAction *>	_actions;
	std::vector<QWindow *>	_windows;
public:
	WindowsMenu(QMenu *menu);
	virtual ~WindowsMenu();
	static WindowsMenu	*get();
	void	add(QWidget *widget, QString title);
	void	remove(QWidget *widget);
public slots:
	void	removeEntry();
	void	triggered();
	void	setText(const QString&);
};

} // namespace snowgui

#endif /* _WindowsMenu_h */
