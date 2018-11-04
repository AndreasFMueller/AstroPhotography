/*
 * WindowsMenu.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "WindowsMenu.h"
#include <QMenu>
#include <QAction>
#include <AstroDebug.h>
#include <QApplication>

namespace snowgui {

static WindowsMenu	*_windowsmenu = NULL;

WindowsMenu::WindowsMenu(QMenu *menu) : QObject(NULL), _menu(menu) {
	if (_windowsmenu) {
		throw std::runtime_error("try to create a second windows menu");
	}
	_windowsmenu = this;
}

WindowsMenu::~WindowsMenu() {
	_windowsmenu = NULL;
}

WindowsMenu	*WindowsMenu::get() {
	if (_windowsmenu) {
		return _windowsmenu;
	}
	throw std::runtime_error("no windows menu created yet");
}

void	WindowsMenu::add(QWidget *widget, QString title) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating new menu entry: '%s'",
		title.toLatin1().data());

	// get the associated window
	QWindow	*window = widget->windowHandle();

	// get the title to use
	QString	t = title;
	if (window) {
		t = window->title();
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no window");
	}

	// create a new action
	
	QAction	*action = new QAction(t, this);

	// connect signals
	connect(action, SIGNAL(triggered()), this, SLOT(triggered()));
	connect(widget, SIGNAL(destroyed()), this, SLOT(removeEntry()));
	connect(window, SIGNAL(windowTitleChanged(const QString&)),
		this, SLOT(setText(const QString&)));

	// add the action to the menu
	_menu->addAction(action);

	// remember all those objects
	_widgets.push_back(widget);
	_actions.push_back(action);
	_windows.push_back(window);
}

void	WindowsMenu::removeEntry() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "remove entry triggered");
	QWidget	*_victim = (QWidget *)sender();
	remove(_victim);
}

void	WindowsMenu::remove(QWidget *_victim) {
	for (unsigned int i = 0; i < _widgets.size(); i++) {
		if (_victim == _widgets[i]) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "widget found");
			_widgets.erase(_widgets.begin() + i);
			auto	actionp = _actions.begin() + i;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "menu has %d actions",
				_menu->actions().count());
			_menu->removeAction(*actionp);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "menu now has %d actions",
				_menu->actions().count());
			_actions.erase(actionp);
			// do not delete, the menu already has taken ownership
			// of the action
			//delete *actionp;
			auto	windowp = _windows.begin() + i;
			_windows.erase(windowp);
			return;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "widget not found");
}

void	WindowsMenu::triggered() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window action triggered");
	QAction	*trigger = (QAction *)sender();
	for (unsigned int i = 0; i < _actions.size(); i++) {
		if (_actions[i] == trigger) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "raising window");
			_windows[i]->showNormal();
			_widgets[i]->raise();
		}
	}
}

void	WindowsMenu::setText(const QString& text) {
	QWindow	*trigger = (QWindow *)sender();
	for (unsigned int i = 0; i < _actions.size(); i++) {
		if (_windows[i] == trigger) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "raising window");
			_actions[i]->setText(text);
		}
	}
}

} // namespace snowgui
