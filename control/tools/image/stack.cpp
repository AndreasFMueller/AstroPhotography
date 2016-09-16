/**
 * stack.cpp -- test program for stacking
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroStacking.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroUtils.h>

using namespace astro;
using namespace astro::image;
using namespace astro::image::stacking;
using namespace astro::io;

namespace astro {
namespace app {
namespace stack {

static struct option	longopts[] = {
/* name			argument?		int*	int */
{ "debug",		no_argument,		NULL,	'd' }, /* 0 */
{ "help",		no_argument,		NULL,	'h' }, /* 1 */
{ "output",		required_argument,	NULL,	'o' }, /* 2 */
{ "number",		required_argument,	NULL,	'n' }, /* 3 */
{ "patchsize",		required_argument,	NULL,	'p' }, /* 4 */
{ "searchradius",	required_argument,	NULL,	's' }, /* 5 */
{ "transform",		required_argument,	NULL,	't' }, /* 6 */
{ NULL,			0,			NULL,	 0  }
};

static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ -dh? ] [ -o outfile ] "
		"files..." << std::endl;
	std::cout << std::endl;
	std::cout << "stack a set of images to produce a target image. The "
		"file name arguments" << std::endl;
	std::cout << "are interpreted as FITS images to be stacked. "
		"All images are aligned with" << std::endl;
	std::cout << "the first image in the list and added to it. "
		"The resulting image is then" << std::endl;
	std::cout << "output to the output file." << std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -d,--debug             increase debug level" << std::endl;
	std::cout << " -n,--number=<n>        number of stars to evaluate" << std::endl;
	std::cout << " -o,--output=<outfile>  filename of output file" << std::endl;
	std::cout << " -p,--patchsize=<s>     use patch size <s> for translation analysis" << std::endl;
	std::cout << " -s,--searchradius=<s>  use radius <s> when searching for stars" << std::endl;
	std::cout << " -t,--transform         don't transform the images when stacking" << std::endl;
	std::cout << " -h,-?,--help           display this help" << std::endl;
}

/**
 * \brief Main method for the stacker program
 */
int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	const char	*outfilename = NULL;
	int	patchsize = 256;
	int	numberofstars = 20;
	int	searchradius = 10;
	bool	notransform = false;
	while (EOF != (c = getopt_long(argc, argv, "dh?o:p:n:s:t", longopts,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'n':
			numberofstars = std::stoi(optarg);
			break;
		case 'o':
			outfilename = optarg;
			break;
		case 'p':
			patchsize = std::stoi(optarg);
			break;
		case 's':
			searchradius = std::stoi(optarg);
			break;
		case 't':
			notransform = true;
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		}
	}

	// make sure we have at least 2 files
	if (2 > (argc - optind)) {
		std::cerr << "must specify at least two image files";
		std::cerr << std::endl;
		return EXIT_FAILURE;
	}

	// open the first image, which will serve as the base image
	FITSin	in(argv[optind++]);
	ImagePtr	baseimage = in.read();

	// create a stacker based in the base image
	StackerPtr	stacker = Stacker::get(baseimage);

	// set the parameters from the command line
	stacker->patchsize(patchsize);
	stacker->numberofstars(numberofstars);
	stacker->searchradius(searchradius);
	stacker->notransform(notransform);

	// read all the images
	while (optind < argc) {
		FITSin	in(argv[optind++]);
		ImagePtr	image = in.read();
		stacker->add(image);
	}

	// now do the stacking
	ImagePtr	stackedimage = stacker->image();

	// write the result image
	if(NULL != outfilename) {
		FITSout	out(outfilename);
		out.write(stackedimage);
	} else {
		std::cerr << "no output filename, not writing result image"
			 << std::endl;
	}

	// that's it
	return EXIT_SUCCESS;
}

} // namespace stack
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::stack::main>(argc, argv);
}
