/*
 * unsharp.cpp -- unsharp masking program
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include <AstroAdapter.h>

namespace astro {
namespace app {
namespace unsharp {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' }, /* 1 */
{ "help",	no_argument,		NULL,		'h' }, /* 2 */
{ "radius",     required_argument,	NULL,		'r' },
{ "amount",     required_argument,	NULL,		'a' },
{ NULL,		0,			NULL,		 0  }
};

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ -dh?f ] infile outfile";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug             increase debug level";
	std::cout << std::endl;
	std::cout << "  -f,--force             force overwriting of existing files";
	std::cout << std::endl;
	std::cout << "  -h,--help              show this help message and exit";
	std::cout << std::endl;
	std::cout << "  -r,--radius=<r>        unsharp radius";
	std::cout << std::endl;
	std::cout << "  -a,--amount=<a>        amount of unsharp mask to subtract";
	std::cout << std::endl;
}

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	bool	force = false;
	double	radius = 1;
	double	amount = 0;
	while (EOF != (c = getopt_long(argc, argv, "a:dh?fr:", longopts,
		&longindex))) {
		switch (c) {
		case 'a':
			amount = std::stod(optarg);
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			force = true;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'r':
			radius = std::stod(optarg);
			break;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// make sure we have at least 2 files
	if (optind >= argc) {
		std::cerr << "must specify image to get unsharp" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	infile(argv[optind++]);
	if (optind >= argc) {
		std::cerr << "must specify output file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfile(argv[optind++]);

	// open the input file
	io::FITSin	in(infile);
	ImagePtr	image = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s-image of type %s", 
		image->size().toString().c_str(),
		demangle(image->pixel_type().name()).c_str());

	// unsharp masking of image
	ImagePtr	outimage = adapter::unsharp(image, radius, amount);

	// find out whether the file exists
	io::FITSout	out(outfile);
	if (out.exists()) {
		if (force) {
			out.unlink();
		} else {
			std::cerr << "file " << outfile << " exists" << std::endl;
			return EXIT_FAILURE;
		}
	}

	// write the file
	out.write(outimage);

	return EXIT_SUCCESS;
}

} // namespace unsharp
} // namespace app
} // namespace astro

int     main(int argc, char *argv[]) {
        return astro::main_function<astro::app::unsharp::main>(argc, argv);
}

