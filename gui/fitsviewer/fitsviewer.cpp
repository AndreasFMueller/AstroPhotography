/*
 * fitsviewer.cpp -- small fits image viewer application
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QApplication>
#include <string.h>
#include <AstroDebug.h>
#include <AstroViewer.h>
#include <iostream>
#include "fitsviewerwindow.h"

using namespace astro::image;

namespace astro {

int	main(int argc, char *argv[]) {
	int	c;
	debugtimeprecision = 3;
	
	while (EOF != (c = getopt(argc, argv, "d")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		}

	// the remaining argument is the fits file we want to view
	if (argc <= optind) {
		std::cerr << "missing file name argument" << std::endl;
		return EXIT_SUCCESS;
	}
	std::string	filename(argv[optind]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s", filename.c_str());

	// now initialize the GUI stuff
        QApplication    app(argc, argv);
	FITSViewerWindow	mainwindow(NULL, filename);
	mainwindow.show();
        return app.exec();
}

} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "fitsviewer terminated by exception:" << x.what()
			<< std::endl;
	}
}
