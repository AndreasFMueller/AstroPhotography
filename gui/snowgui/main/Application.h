/*
 * Application.h -- the application class
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _Application_h
#define _Application_h

#include <QApplication>

namespace snowgui {

class Application final : public QApplication {
public:
	Application(int& argc, char **argv) : QApplication(argc, argv) { }
	virtual bool	notify(QObject *receiver, QEvent *e) override;
};

} // namespace snowgui

#endif /* _Application_h */
