/*
 * fold.cpp -- tool to perform the folding operation as a demonstration for
 *             the Numb3rs-lecture on ergodic theory
 *
 * (c) 2026 Prof Dr Andreas Müller
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include <AstroAdapter.h>
#include <AstroPixel.h>
#include <AstroFilterfunc.h>

namespace astro {
namespace app {
namespace fold {

static bool	force = false;
static bool	maximum = false;
static float	scale = 1.0f;

/**
 * \brief base class for fold adapters
 */
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

/**
 * \brief Horizontal folder adapter
 */
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
		Pixel	p1 = FoldAdapter<Pixel>::_image.pixel(xx, y) * 0.5;
		Pixel	p2 = FoldAdapter<Pixel>::_image.pixel(w - xx, y) * 0.5;
		Pixel	s = p1 + p2;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "(%d,%d) -> %s/%s -> %s", x, y,
			astro::image::pixelValueString(p1).c_str(),
			astro::image::pixelValueString(p2).c_str(),
			astro::image::pixelValueString(s).c_str());
		return s;
	}
};

/**
 * \brief Vertical folder adapter
 */
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
		Pixel	p1 = FoldAdapter<Pixel>::_image.pixel(x, yy) * 0.5;
		Pixel	p2 = FoldAdapter<Pixel>::_image.pixel(x, h - yy) * 0.5;
		Pixel	s = p1 + p2;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "(%d,%d) -> %s/%s -> %s", x, y,
			astro::image::pixelValueString(p1).c_str(),
			astro::image::pixelValueString(p2).c_str(),
			astro::image::pixelValueString(s).c_str());
		return s;
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
 * \brief Create a test image
 *
 * \param size		size of the image
 * \param color		whether the image is a color image
 * \param integer	whether or not the image should use an
 *                      integer pixel type
 */
static ImagePtr		create_testimage(const ImageSize& size, bool color,
				bool integer) {
	ImagePtr	result;
	int	xu = size.width() / 16;
	int	yu = size.height() / 4;
	if (color) {
		if (integer) {
			Image<RGB<unsigned short>>	*image
				= new Image<RGB<unsigned short>>(size);
			result = ImagePtr(image);
			RGB<unsigned short>	pv((unsigned short)0,
							(unsigned short)0,
							(unsigned short)0);
			image->fill(pv);
			// add stuff
			{
				pv.R = 32767;
				image->fill(ImageRectangle(
					ImagePoint(xu, yu),
					ImageSize(2 * xu, 2 * yu)),
					pv);
			}
			{
				pv.R = 0;
				pv.G = 65535;
				for (int x = 5 * xu; x < 7 * xu; x += 2) {
					image->fill(ImageRectangle(
						ImagePoint(x, yu),
						ImageSize(1, 2 * yu)),
						pv);
				}
			}
			{
				pv.G = 0;
				pv.B = 65535;
				for (int y = yu; y < 3 * yu; y += 2) {
					image->fill(ImageRectangle(
						ImagePoint(9 * xu, y),
						ImageSize(2 * xu, 1)),
						pv);
				}
			}
			{
				pv.R = 65535;
				pv.G = 65535;
				for (int x = 13 * xu; x < 15 * xu; x++) {
					float	xx = (x - 14. * xu) / xu;
					for (int y = yu; y < 3 * yu; y++) {
						float	yy = (y - 2. * yu) / yu;
						float	r = hypotf(xx, yy);
						if (r < 1) {
							image->writablepixel(x, y) = pv;
						}
					}
				}
			}
		} else {
			Image<RGB<float>>	*image
				= new Image<RGB<float>>(size);
			result = ImagePtr(image);
			RGB<float>	pv(0.0f, 0.0f, 0.0f);
			image->fill(pv);
			// add stuff
			{
				pv.R = 0.5f;
				image->fill(ImageRectangle(
					ImagePoint(xu, yu),
					ImageSize(2 * xu, 2 * yu)),
					pv);
			}
			{
				pv.R = 0.0f;
				pv.G = 1.0f;
				for (int x = 5 * xu; x < 7 * xu; x += 2) {
					image->fill(ImageRectangle(
						ImagePoint(x, yu),
						ImageSize(1, 2 * yu)),
						pv);
				}
			}
			{
				pv.G = 0.0f;
				pv.B = 1.0f;
				for (int y = yu; y < 3 * yu; y += 2) {
					image->fill(ImageRectangle(
						ImagePoint(9 * xu, y),
						ImageSize(2 * xu, 1)),
						pv);
				}
			}
			{
				pv.R = 1.0f;
				pv.G = 1.0f;
				for (int x = 13 * xu; x < 15 * xu; x++) {
					float	xx = (x - 14. * xu) / xu;
					for (int y = yu; y < 3 * yu; y++) {
						float	yy = (y - 2. * yu) / yu;
						float	r = hypotf(xx, yy);
						if (r < 1) {
							image->writablepixel(x, y) = pv;
						}
					}
				}
			}
		}
	} else {
		if (integer) {
			Image<unsigned short>	*image
				= new Image<unsigned short>(size);
			result = ImagePtr(image);
			image->fill((unsigned short)0);
			// add stuff
			unsigned short	pv = 65535;
			image->fill(ImageRectangle(
				ImagePoint(xu, yu), ImageSize(2 * xu, 2 * yu)),
				pv);
			pv = pv / 2;
			for (int x = 5 * xu; x < 7 * xu; x += 2) {
				image->fill(ImageRectangle(
					ImagePoint(x, yu),
					ImageSize(1, 2 * yu)),
					pv);
			}
			pv = pv / 2;
			for (int y = yu; y < 3 * yu; y += 2) {
				image->fill(ImageRectangle(
					ImagePoint(9 * xu, y),
					ImageSize(2 * xu, 1)),
					pv);
			}
			pv = pv / 2;
			for (int x = 13 * xu; x < 15 * xu; x++) {
				for (int y = yu; y < 3 * yu; y++) {
					float	r = hypotf((x - 14.f * xu) / xu,
							(y - 2.f * yu) / yu);
					if (r < 1) {
						image->writablepixel(x, y) = pv;
					}
				}
			}
		} else {
			Image<float>	*image
				= new Image<float>(size);
			result = ImagePtr(image);
			image->fill(0.0f);
			// add stuff
			float	pv = 1.0f;
			image->fill(ImageRectangle(
				ImagePoint(xu, yu), ImageSize(2 * xu, 2 * yu)),
				pv);
			pv = pv / 2;
			for (int x = 5 * xu; x < 7 * xu; x += 2) {
				image->fill(ImageRectangle(
					ImagePoint(x, yu),
					ImageSize(1, 2 * yu)),
					pv);
			}
			pv = pv / 2;
			for (int y = yu; y < 3 * yu; y += 2) {
				image->fill(ImageRectangle(
					ImagePoint(9 * xu, y),
					ImageSize(2 * xu, 1)),
					pv);
			}
			pv = pv / 2;
			for (int x = 13 * xu; x < 15 * xu; x++) {
				for (int y = yu; y < 3 * yu; y++) {
					float	r = hypotf((x - 14.f * xu) / xu,
							(y - 2.f * yu) / yu);
					if (r < 1) {
						image->writablepixel(x, y) = pv;
					}
				}
			}
		}
	}
	return result;
}

/**
 * \brief Options for the fold program
 */
static struct option	longopts[] = {
{ "both",	no_argument,		NULL,		'b' },
{ "color",	no_argument,		NULL,		'c' },
{ "debug",	no_argument,		NULL,		'd' },
{ "force",	no_argument,		NULL,		'f' },
{ "help",	no_argument,		NULL,		'H' },
{ "height",	required_argument,	NULL,		'h' },
{ "integer",	no_argument,		NULL,		'i' },
{ "maximum",	no_argument,		NULL,		'm' },
{ "repeat",	required_argument,	NULL,		'r' },
{ "sequence",	no_argument,		NULL,		'n' },
{ "scale",	required_argument,	NULL,		's' },
{ "test",	no_argument,		NULL,		't' },
{ "vertical",	no_argument,		NULL,		'v' },
{ "width",	required_argument,	NULL,		'w' },
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
	std::cout << "    " << path.basename() << " [ -bdfHh?vw ] image folded";
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << " -b,--both          fold in both directions" << std::endl;
	std::cout << " -c,--color         generate a color test image"
		<< std::endl;
	std::cout << " -d,--debug         increase debug level" << std::endl;
	std::cout << " -f,--force         force overwriting output file"
		 << std::endl;
	std::cout << " -h,--height=<h>    height of the test image"
		<< std::endl;
	std::cout << " -i,--integer       use an integer pixel type for the "
		"test image" << std::endl;
	std::cout << " -H,-?,--help       show this help message and exit"
		<< std::endl;
	std::cout << " -m,--maximaum      rescale image to maximum pixel value"
		<< std::endl;
	std::cout << " -r,--repeat=<r>    repeat folding <r> times"
		<< std::endl;
	std::cout << " -n,--sequence      create a output file sequence"
		<< std::endl;
	std::cout << " -t,--test          create a test image" << std::endl;
	std::cout << " -v,--vertical      fold vertically" << std::endl;
	std::cout << " -w,--width=<w>     width of the test image" << std::endl;
	std::cout << std::endl;
}

#define rescale_pixels(image, scale, pixel)				\
{									\
	Image<pixel>	*p = dynamic_cast<Image<pixel >*>(&*image);	\
	if (NULL != p) {						\
		adapter::RescalingAdapter<pixel>	ra(*p, 0, scale);\
		adapter::ConvertingAdapter<RGB<unsigned char>, pixel>	ca(ra);\
		scaledimage = new Image<RGB<unsigned char>> (ca);	\
	}								\
}

/**
 * \brief write the image to a file
 *
 * \param image		the image to write
 * \param filename	the base filename
 * \param number	the number of the file
 */
static void	writeImage(ImagePtr image, const std::string& filename,
			int number) {
	// extract the extension for the image type
	auto pointoffset = filename.find_last_of(".");
	std::string	name = filename.substr(0, pointoffset);
	std::string	extension = filename.substr(pointoffset + 1);

	// add format the number into the filename
	std::string	outfilename;
	if (number >= 0) {
		outfilename = stringprintf("%s-%03d.%s", name.c_str(), number,
			extension.c_str());
	} else {
		outfilename = filename;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing image to file %s, extension = %s",
		outfilename.c_str(), extension.c_str());

	// detect FITS image
	if (FITS::isfitsfilename(outfilename)) {
		io::FITSout	out(outfilename);
		if (out.exists()) {
			if (force) {
				out.unlink();
			} else {
				std::string	msg = stringprintf("file %s "
					"exists", outfilename.c_str());
				debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
				throw std::runtime_error(msg);
			}
		}
		out.write(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image written to %s",
			outfilename.c_str());
		return;
	}

	if (maximum) {
		// find the scale factor
		scale = astro::image::filter::max_luminance(image);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found maximum = %f", scale);
	}
	scale = 255. / scale;
	Image<RGB<unsigned char>>	*scaledimage = NULL;
	rescale_pixels(image, scale, unsigned char)
	rescale_pixels(image, scale, unsigned short)
	rescale_pixels(image, scale, unsigned int)
	rescale_pixels(image, scale, unsigned long)
	rescale_pixels(image, scale, float)
	rescale_pixels(image, scale, double)
	rescale_pixels(image, scale, RGB<unsigned char>)
	rescale_pixels(image, scale, RGB<unsigned short>)
	rescale_pixels(image, scale, RGB<unsigned int>)
	rescale_pixels(image, scale, RGB<unsigned long>)
	rescale_pixels(image, scale, RGB<float>)
	rescale_pixels(image, scale, RGB<double>)
	if (NULL == scaledimage) {
		std::string	msg = stringprintf("unknown pixel type");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	
	ImagePtr	scaledimageptr(scaledimage);

	// detect PNG image
	if (PNG::ispngfilename(outfilename)) {
		// write PNG 
		debug(LOG_DEBUG, DEBUG_LOG, 0, "writing PNG image to %s",
			outfilename.c_str());
		PNG().writePNG(scaledimageptr, outfilename);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "PNG image %s written",
			outfilename.c_str());
	}

	// detect JPG image
	if (JPEG::isjpegfilename(outfilename)) {
		// write a JPEG image
		debug(LOG_DEBUG, DEBUG_LOG, 0, "writing JPEG image to %s",
			outfilename.c_str());
		JPEG().writeJPEG(scaledimageptr, outfilename);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "JPEG image %s written",
			outfilename.c_str());
	}
}

/**
 * \brief Main function for the fold tool
 *
 * \param argc	number of arguments
 * \param argv	arguments
 */
int	main(int argc, char *argv[]) {
	// boolean control variables
	bool	vertical = false;
	bool	both = false;
	int	repeat = 1;
	bool	testimage = false;
	bool	color = false;
	bool	integer = false;
	int	width = 64;
	int	height = 36;
	int	sequencenumber = -1;

	// parse command line
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "bcdfh?irstvw", longopts,
		&longindex))) {
		switch (c) {
		case 'b':
			both = true;
			break;
		case 'c':
			color = true;
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'f':
			force = true;
			break;
		case 'h':
			height = std::stoi(optarg);
			break;
		case 'i':
			integer = true;
			break;
		case 'H':
		case '?':
			usage(argv[0]);
			return EXIT_SUCCESS;
		case 'm':
			maximum = true;
			break;
		case 'n':
			sequencenumber = 0;
			break;
		case 'r':
			repeat = std::stoi(optarg);
			break;
		case 's':
			scale = std::stod(optarg);
			break;
		case 't':
			testimage = true;
			break;
		case 'v':
			vertical = true;
			break;
		case 'w':
			width = std::stoi(optarg);
			break;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// image file names
	std::string	infilename;
	if (testimage) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "creating a test image");
	} else {
		if (optind >= argc) {
			std::cerr << "must specify input image file name"
				<< std::endl;
			return EXIT_FAILURE;
		}
		infilename = std::string(argv[optind++]);
	}
	if (optind >= argc) {
		std::cerr << "must specify folded image file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfilename(argv[optind++]);

	// show what we are doing
	debug(LOG_DEBUG, DEBUG_LOG, 0, "fold %s %s into %s",
		(testimage) ? "testimage" : infilename.c_str(), 
		(vertical) ? "vertically" : "horizontally",
		outfilename.c_str());

	// open input FITS image
	ImagePtr	image;
	if (testimage) {
		// generate a test image
		image = create_testimage(ImageSize(width, height),
			color, integer);
	} else {
		io::FITSin	in(infilename);
		image = in.read();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s-image of type %s",
		image->size().toString().c_str(),
		demangle(image->pixel_type().name()).c_str());

	// perform the fold
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start folding");
	ImagePtr	result = image;
	if (sequencenumber >= 0) {
		writeImage(result, outfilename, sequencenumber++);
	}
	while (repeat-- > 0) {
		if (!vertical || both) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "horizontal fold");
			fold_adapter(result, double, false);
			fold_adapter(result, float, false);
			fold_adapter(result, unsigned short, false);
			fold_adapter(result, unsigned long, false);
			fold_adapter(result, unsigned char, false);
			fold_adapter(result, RGB<double>, false);
			fold_adapter(result, RGB<float>, false);
			fold_adapter(result, RGB<unsigned short>, false);
			fold_adapter(result, RGB<unsigned long>, false);
			fold_adapter(result, RGB<unsigned char>, false);
			if (sequencenumber >= 0) {
				writeImage(result, outfilename,
					sequencenumber++);
			}
		}
		if (vertical || both) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "vertical fold");
			fold_adapter(result, double, true);
			fold_adapter(result, float, true);
			fold_adapter(result, unsigned short, true);
			fold_adapter(result, unsigned long, true);
			fold_adapter(result, unsigned char, true);
			fold_adapter(result, RGB<double>, true);
			fold_adapter(result, RGB<float>, true);
			fold_adapter(result, RGB<unsigned short>, true);
			fold_adapter(result, RGB<unsigned long>, true);
			fold_adapter(result, RGB<unsigned char>, true);
			if (sequencenumber >= 0) {
				writeImage(result, outfilename,
					sequencenumber++);
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "folding complete");

	// write output FITS image
	if (sequencenumber < 0) {
		writeImage(result, outfilename, sequencenumber);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "folded image %s written",
			outfilename.c_str());
	}

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
