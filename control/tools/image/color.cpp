/*
 * color.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <AstroIO.h>
#include <includes.h>
#include <limits>

namespace astro {
namespace app {
namespace color {

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage:" << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ options ] infile outfile";
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -d,--debug        increase debug level" << std::endl;
	std::cout << "  -f,--force        force overwriting of existing files";
	std::cout << std::endl;
	std::cout << "  -h,-?,--help      show this help message and exit";
	std::cout << std::endl;
	std::cout << "  -s,--scales=<s>   set color scale factors (comma separated values)" << std::endl;
	std::cout << "  -o,--offsets=<o>  set the color offsets (comma seprated values)" << std::endl;
	std::cout << "  -g,--gain=<g>     set the gain" << std::endl;
	std::cout << "  -b,--base=<b>     base value of the color scale";
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "base",	required_argument,	NULL,	'b' },
{ "debug",	no_argument,		NULL,	'd' },
{ "force",	no_argument,		NULL,	'f' },
{ "gain",	required_argument,	NULL,	'g' },
{ "help",	no_argument,		NULL,	'h' },
{ "limit",	required_argument,	NULL,	'l' },
{ "offsets",	required_argument,	NULL,	'o' },
{ "scales",	required_argument,	NULL,	's' },
{ NULL,		0,			NULL,	 0  }
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	bool	force = false;
	adapter::ColorTransformBase	colorbase;
	while (EOF != (c = getopt_long(argc, argv, "b:dfg:h?l:o:s:", longopts,
		&longindex)))
		switch (c) {
		case 'b':
			colorbase.base(std::stod(optarg));
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			force = true;
			break;
		case 'g':
			colorbase.gain(std::stod(optarg));
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'l':
			colorbase.limit(std::stod(optarg));
			break;
		case 's':
			colorbase.scales(std::string(optarg));
			break;
		case 'o':
			colorbase.offsets(std::string(optarg));
			break;
		default:
			throw std::runtime_error("unknown option");
	}

	// make sure we have two files
	if (optind >= argc) {
		std::cerr << "must specify file to color edit" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	infile(argv[optind++]);
	if (optind >= argc) {
		std::cerr << "must specify output file" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfile(argv[optind++]);

	// read the input fiel
	io::FITSin	in(infile);
	ImagePtr	image = in.read();
	if (3 != image->planes()) {
		std::cerr << "not a color image" << std::endl;
		return EXIT_FAILURE;
	}

	// process the image
	ImagePtr	outimage = adapter::colortransform(image, colorbase);

	// write the output file
	io::FITSout	out(outfile);
	if ((out.exists()) && (force)) {
		out.unlink();
	}
	out.write(outimage);


	return EXIT_SUCCESS;
}

} // namespace color
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::color::main>(argc, argv);
}
