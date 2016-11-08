/*
 * fitsheader.cpp -- fits header manipulation utility
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <stdexcept>
#include <includes.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroUtils.h>
#include <iostream>
#include <fitsio.h>
#include <string>
#include <typeinfo>

namespace astro {
namespace app {
namespace fitsheader {

/**
 * \brief Display all headers of a fits file
 */
void	display_headers(fitsfile *fits) {
	int	status = 0;
	int	keynum = 1;
	char	keyname[100];
	char	value[100];
	char	comment[100];
	while (1) {
		if (fits_read_keyn(fits, keynum, keyname, value, comment,
			&status)) {
			return;
		}

		// display the header just found:
		std::cout << stringprintf("%-8.8s = %s / %s",
			keyname, value, comment) << std::endl;
		keynum++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "headers displayed");
}

/**
 * \brief Delete a header from a FITS file
 */
void	delete_header(fitsfile *fits, const char *headername) {
	int	status = 0;
	char	fitserrmsg[80];
	if (fits_delete_key(fits, headername, &status)) {
		fits_get_errstatus(status, fitserrmsg);
		std::string	msg = stringprintf("cannot delete header: %s",
			fitserrmsg);
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "header '%s' deleted", headername);
}

/**
 * \brief Add a header to a FITS file
 */
void	add_header(fitsfile *fits, const char *key, const char *value,
		const char *comment) {
	int	status = 0;
	char	fitserrmsg[80];

	// first try integer type
	try {
		long	ivalue = std::stol(std::string(value));
		double	dvalue = std::stod(std::string(value));

		if (ivalue == dvalue) {
			if (fits_write_key(fits, TLONG, (char *)key,
				(char *)&ivalue, (char *)comment, &status)) {
				fits_get_errstatus(status, fitserrmsg);
				std::string msg = stringprintf(
					"cannot add '%s': %s", key, fitserrmsg);
				throw std::runtime_error(msg);
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "added int header");
		} else {
			if (fits_write_key(fits, TDOUBLE, (char *)key,
				(char *)&dvalue, (char *)comment, &status)) {
				fits_get_errstatus(status, fitserrmsg);
				std::string msg = stringprintf(
					"cannot add '%s': %s", key, fitserrmsg);
				throw std::runtime_error(msg);
			}
			debug(LOG_DEBUG, DEBUG_LOG, 0, "added double header");
		}
		return;
	} catch (...) {
	}

	// if either failed, add as string header
	if (fits_write_key(fits, TSTRING, (char *)key, (char *)value,
		(char *)comment, &status)) {
		fits_get_errstatus(status, fitserrmsg);
		std::string	msg = stringprintf("cannot add '%s': %s",
					key, fitserrmsg);
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "header '%s' added", key);
}

static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] display <file.fits>" << std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] add <file.fits> <key> <value> ..."
		<< std::endl;
	std::cout << "    " << p.basename();
	std::cout << " [ options ] delete <file.fits> <key> <value>"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << std::endl;
	std::cout << "    -d,--debug       increase debug level" << std::endl;
	std::cout << "    -h,-?,--help     display this help message" << std::endl;
}

static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,	'd' }, /* 0 */
{ "help",		no_argument,		NULL,	'h' }, /* 1 */
{ NULL,			0,			NULL,	 0  },
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "d", longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		default:
			throw std::runtime_error("unknown option");
		}

	// next argument must be the command
	if (optind >= argc) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	command(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "command: %s", command.c_str());

	// find out whether we need to write the file
	bool	readonly = false;
	if (command == "display") {
		readonly = true;
	}

	// next argument must be the image file
	if (optind >= argc) {
		throw std::runtime_error("not enough arguments");
	}
	std::string	filename(argv[optind++]);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s", filename.c_str());

	// open the FITS file
	char	fitserrmsg[80];
	int	status = 0;
	fitsfile	*fits = NULL;
	if (fits_open_file(&fits, filename.c_str(),
		(readonly) ? READONLY : READWRITE, &status)) {
		fits_get_errstatus(status, fitserrmsg);
		std::string	msg = stringprintf("FITS error: %s",
					fitserrmsg);
		throw std::runtime_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file opened");

	// execute the command
	if (command == "display") {
		display_headers(fits);
	}
	if (command == "delete") {
		while (optind < argc) {
			delete_header(fits, argv[optind++]);
		}
	}
	if (command == "add") {
		while (optind + 2 < argc) {
			add_header(fits, argv[optind], argv[optind + 1],
				argv[optind + 2]);
			optind += 3;
		}
	}

	// close the file again
	fits_close_file(fits, &status);

	return EXIT_SUCCESS;
}

} // namespace fitsheader
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::fitsheader::main>(argc, argv);
}


