/*
 * astroviewer.cpp -- main function for the snowgui application
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <browserwindow.h>
#include <QApplication>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <AstroConfig.h>
#include <AstroIO.h>
#include <getopt.h>
#include <QFileDialog>
#include <imagedisplaywidget.h>

namespace snowgui {
namespace viewer {

/**
 * \brief Usage function for the snowgui program
 */
static void	usage(const char *progname) {
	astro::Path	path(progname);
	std::cout << "usage:" << std::endl;
	std::cout << "    " << path.basename() << " [ options ]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug          increase debug level" << std::endl;
	std::cout << "  -h,-?,--help        show this help message and exit"
		<< std::endl;
}

/**
 * \brief command line options
 */
static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,	'd' },
{ "help",		no_argument,		NULL,	'h' },
{ NULL,			0,			NULL,	 0  }
};

/**
 * \brief Main-function of the snowgui program
 */
int main(int argc, char *argv[]) {
	// debug initialization
	debug_set_ident("snowgui");
	debugthreads = 1;

	// parse the command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "dh?",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test program starting up");

	// start the application
	QApplication a(argc, argv);
	a.setApplicationDisplayName(QString("Browser"));

	// create a new browser an a directory
	std::string	filename;
	if (optind < argc) {
		filename = std::string(argv[optind++]);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "open file %s",
			filename.c_str());
	} else {
		QFileDialog     filedialog;
		filedialog.setAcceptMode(QFileDialog::AcceptOpen);
		//filedialog.setFileMode(QFileDialog::DirectoryOnly);
		if (!filedialog.exec()) {
			return EXIT_FAILURE;
		}
		QStringList     list = filedialog.selectedFiles();
		filename = std::string(list.begin()->toLatin1().data());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "directory: %s",
			filename.c_str());
	}
	ImagePtr	image;
	// read the file
	astro::io::FITSin	in(filename);
	try {
		image = in.read();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%s: %s", filename.c_str(),
			x.what());
	}
	astro::Path	p(filename);

	// open the image
	imagedisplaywidget	*idw = new imagedisplaywidget(NULL);
	idw->setImage(image);
	idw->setWindowTitle(QString(p.basename().c_str()));
	idw->show();

	return a.exec();
}

} // namespace viewer
} // namespace snowgui

// wrapper used to catch and log any exceptions
int	main(int argc, char *argv[]) {
	return astro::main_function<snowgui::viewer::main>(argc, argv);
}
