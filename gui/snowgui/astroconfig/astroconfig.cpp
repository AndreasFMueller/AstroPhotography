/*
 * astroconfig.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <QApplication>
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <configurationwidget.h>

namespace snowgui {

int	main(int argc, char *argv[]) {
	debug_set_ident("snowgui");
	debugthreads = 1;
	debuglevel = LOG_DEBUG;

	QApplication	a(argc, argv);
	a.setApplicationDisplayName(QString("Configuration"));

	snowgui::configurationwidget	*cw = new configurationwidget(NULL);
	cw->filltable();
	cw->setWindowTitle("Configuration");
	cw->show();
	
	a.exec();
}

} // namespace snowgui

int	main(int argc, char *argv[]) {
	return astro::main_function<snowgui::main>(argc, argv);
}
