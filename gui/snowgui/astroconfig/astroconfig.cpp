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
#include <getopt.h>

namespace snowgui {

/**
 * \brief Command line options for the local configuration program
 */
static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,	'd' },
{ NULL,			0,			NULL,	 0  }
};

int	main(int argc, char *argv[]) {
	debug_set_ident("snowgui");
	debugthreads = 1;

	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "d", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	QApplication	a(argc, argv);
	a.setApplicationDisplayName(QString("Configuration"));

	snowgui::configurationwidget	*cw = new configurationwidget(NULL);
	cw->filltable();
	cw->setWindowTitle("Configuration");
	cw->show();
	
	a.exec();
	return EXIT_SUCCESS;
}

} // namespace snowgui

int	main(int argc, char *argv[]) {
	return astro::main_function<snowgui::main>(argc, argv);
}
