/*
 * fold.cpp
 *
 * (c) 2026 Prof Dr Andreas Müller
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include <AstroAdapter.h>
#include <AstroAdapter.h>

namespace astro {
namespace app {
namespace fold {

template<typename Pixel>
class FoldAdapter : public ConstImageAdapter<Pixel> {
protected:
	const ConstImageAdapter<Pixel>&	_image;
public:
	FoldAdapter(const ConstImageAdapter<Pixel>& image)
		: ConstImageAdapter<Pixel>(image.getSize()), _image(image) {
	}
	virtual Pixel	pixel(int x, int y) const {
		Pixel	p;
		return p;
	}
};

template<typename Pixel>
class HorizontalFoldAdapter : public FoldAdapter<Pixel> {
	int	w;
public:
	HorizontalFoldAdapter(const ConstImageAdapter<Pixel>& image)
		: FoldAdapter<Pixel>(image) {
		w = image.getSize().width() - 1;
	}
	virtual Pixel	pixel(int x, int y) const {
		int	xx = x/2;
		Pixel	p1 = FoldAdapter<Pixel>::_image.pixel(xx, y);
		Pixel	p2 = FoldAdapter<Pixel>::_image.pixel(w - xx, y);
		Pixel	s = p1 + p2;
		Pixel	result = s * 0.5;
		return result;
	}
};

template<typename Pixel>
class VerticalFoldAdapter : public FoldAdapter<Pixel> {
	int	h;
public:
	VerticalFoldAdapter(const ConstImageAdapter<Pixel>& image)
		: FoldAdapter<Pixel>(image) {
		h = image.getSize().height() - 1;
	}
	virtual Pixel	pixel(int x, int y) const {
		int	yy = y/2;
		Pixel	p1 = FoldAdapter<Pixel>::_image.pixel(x, yy);
		Pixel	p2 = FoldAdapter<Pixel>::_image.pixel(x, h - yy);
		Pixel	s = p1 + p2;
		Pixel	result = s * 0.5;
		return result;
	}
};

#define fold_adapter(image, pixel, vertical)				\
{									\
	Image<pixel>	*p = dynamic_cast<Image<pixel >*>(&*image);	\
	if (NULL != p) {						\
		if (vertical) {						\
			VerticalFoldAdapter<pixel >	a(*p);		\
			result = ImagePtr(new Image<pixel>(a));		\
		} else {						\
			HorizontalFoldAdapter<pixel >	a(*p);		\
			result = ImagePtr(new Image<pixel>(a));		\
		}							\
	}								\
}

/**
 * \brief Options for the fold program
 */
static struct option	longopts[] = {
{ "both",	no_argument,		NULL,		'b' },
{ "debug",	no_argument,		NULL,		'd' },
{ "force",	no_argument,		NULL,		'f' },
{ "help",	no_argument,		NULL,		'h' },
{ "repeat",	required_argument,	NULL,		'r' },
{ "vertical",	no_argument,		NULL,		'v' },
{ NULL,		0,			NULL,		 0  }
};

/**
 * \brief Usage function for the fold program
 *
 * \param progname	name of the program
 */
static void	usage(const char *progname) {
	Path	path(progname);
	std::cout << "usage: " << std::endl;
	std::cout << std::endl;
	std::cout << "    " << path.basename() << " [ -dhv ] image folded";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--both          fold in both directions" << std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -f,--force         force overwriting output file"
		 << std::endl;
	std::cout << " -h,-?,--help       show this help message and exit"
		<< std::endl;
	std::cout << " -r,--repeat=<r>    repeat folding <r> times"
		<< std::endl;
	std::cout << " -v,--vertical      fold vertically" << std::endl;
	std::cout << std::endl;
}

/**
 * \brief Main function for the fold tool
 *
 * \param argc	number of arguments
 * \param argv	arguments
 */
int	main(int argc, char *argv[]) {
	// boolean control variables
	bool	force = false;
	bool	vertical = false;
	bool	both = false;
	int	repeat = 1;

	// parse command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "bdfh?rv", longopts,
		&longindex))) {
		switch (c) {
		case 'b':
			both = true;
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
			repeat = std::stoi(optarg);
			break;
		case 'v':
			vertical = true;
			break;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// image file names
	if (optind >= argc) {
		std::cerr << "must specify input image file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	infile(argv[optind++]);
	if (optind >= argc) {
		std::cerr << "must specify folded image file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfile(argv[optind++]);

	// show what we are doing
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fold %s %s into %s",
		infile.c_str(), 
		(vertical) ? "vertically" : "horizontally",
		outfile.c_str());

	// open input FITS image
	io::FITSin	in(infile);
	ImagePtr	image = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s-image of type %s",
		image->size().toString().c_str(),
		demangle(image->pixel_type().name()).c_str());

	// perform the fold
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start folding");
	ImagePtr	result;
	while (repeat--) {
		if (!vertical || both) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "horizontal fold");
			fold_adapter(image, double, false);
			fold_adapter(image, float, false);
			fold_adapter(image, unsigned short, false);
			fold_adapter(image, unsigned long, false);
			fold_adapter(image, unsigned char, false);
			fold_adapter(image, RGB<double>, false);
			fold_adapter(image, RGB<float>, false);
			fold_adapter(image, RGB<unsigned short>, false);
			fold_adapter(image, RGB<unsigned long>, false);
			fold_adapter(image, RGB<unsigned char>, false);
			image = result;
		}
		if (vertical || both) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "vertical fold");
			fold_adapter(image, double, true);
			fold_adapter(image, float, true);
			fold_adapter(image, unsigned short, true);
			fold_adapter(image, unsigned long, true);
			fold_adapter(image, unsigned char, true);
			fold_adapter(image, RGB<double>, true);
			fold_adapter(image, RGB<float>, true);
			fold_adapter(image, RGB<unsigned short>, true);
			fold_adapter(image, RGB<unsigned long>, true);
			fold_adapter(image, RGB<unsigned char>, true);
			image = result;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "folding complete");

	// write output FITS image
	io::FITSout	out(outfile);
	if (out.exists()) {
		if (force) {
			out.unlink();
		} else {
			std::cerr << "file " << outfile << " exists"
				<< std::endl;
			return EXIT_FAILURE;
		}
	}
	out.write(result);

	// done
	return EXIT_SUCCESS;
}

} // namespace fold
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	try {
		return astro::app::fold::main(argc, argv);
	} catch (std::exception& x) {
		std::cerr << "fold terminated by exception: " << x.what()
			<< std::endl;
	} catch (...) {
		std::cerr << "fold terminated by unknown exception"
			<< std::endl;
	}
	return EXIT_FAILURE;
}
