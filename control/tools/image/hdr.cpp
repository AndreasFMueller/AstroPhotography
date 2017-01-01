/*
 * hdr.cpp -- hdr masking program
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <AstroUtils.h>
#include <AstroIO.h>
#include <AstroAdapter.h>
#include <AstroConvolve.h>

namespace astro {
namespace app {
namespace hdr {

static struct option	longopts[] = {
/* name		argument?		int*		int */
{ "debug",	no_argument,		NULL,		'd' }, /* 0 */
{ "force",	no_argument,		NULL,		'f' }, /* 1 */
{ "deemphasize",required_argument,	NULL,		'e' }, /* 2 */
{ "help",	no_argument,		NULL,		'h' }, /* 3 */
{ "mask",	required_argument,	NULL,		'm' }, /* 4 */
{ "radius",     required_argument,	NULL,		'r' }, /* 5 */
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
	std::cout << "  -e,--demphasize=e      set degree of deimphasizing, typical values are";
	std::cout << std::endl;
	std::cout << " around 0.005";
	std::cout << std::endl;
	std::cout << "  -f,--force             force overwriting of existing files";
	std::cout << std::endl;
	std::cout << "  -h,--help              show this help message and exit";
	std::cout << std::endl;
	std::cout << " -m,--mask=<mask.fits>  use the mask in file mask.fits to deemphasize the";
	std::cout << std::endl;
	std::cout << "                         image";
	std::cout << std::endl;
	std::cout << "  -r,--radius=<r>        blurr radius in the mask before applying it to deemphasize";
	std::cout << std::endl;
}

template<typename T, typename S>
class DeemphasizingAdapter : public ConstImageAdapter<T> {
	const ConstImageAdapter<T>&     _image;
	const ConstImageAdapter<S>&     _deemph;
	double	_degree;
public:
	DeemphasizingAdapter(const ConstImageAdapter<T>& image,
		const ConstImageAdapter<S>& deemph, double degree)
		: ConstImageAdapter<T>(image.getSize()), _image(image),
		  _deemph(deemph), _degree(degree) {
	}
	virtual T       pixel(int x, int y) const {
		double	f = 1 / (_degree * _deemph.pixel(x, y) + 1.);
		return _image.pixel(x, y) * f;
	}
};

#define deemphasize(Pixel, imageptr, blurredmask, degree)		\
{									\
	Image<Pixel>	*image = dynamic_cast<Image<Pixel>*>(&*imageptr);\
	if (image != NULL) {						\
		DeemphasizingAdapter<Pixel, double>	demph(*image,	\
			blurredmask, degree);				\
		return ImagePtr(new Image<Pixel>(demph));		\
	}								\
}

ImagePtr	do_deemphasize(ImagePtr imageptr,
	const ConstImageAdapter<double>& blurredmask, double degree) {
	deemphasize(unsigned char, imageptr, blurredmask, degree);
	deemphasize(unsigned short, imageptr, blurredmask, degree);
	deemphasize(unsigned int, imageptr, blurredmask, degree);
	deemphasize(unsigned long, imageptr, blurredmask, degree);
	deemphasize(float, imageptr, blurredmask, degree);
	deemphasize(double, imageptr, blurredmask, degree);
	deemphasize(RGB<unsigned char>, imageptr, blurredmask, degree);
	deemphasize(RGB<unsigned short>, imageptr, blurredmask, degree);
	deemphasize(RGB<unsigned int>, imageptr, blurredmask, degree);
	deemphasize(RGB<unsigned long>, imageptr, blurredmask, degree);
	deemphasize(RGB<float>, imageptr, blurredmask, degree);
	deemphasize(RGB<double>, imageptr, blurredmask, degree);
	throw std::runtime_error("don't know how to deemphasize this image");
}

#define typeconvert(Pixel, maskptr)					\
{									\
	Image<Pixel>	*maskimage					\
		= dynamic_cast<Image<Pixel>*>(&*maskptr);		\
	if (NULL != maskimage) {					\
		adapter::TypeConversionAdapter<Pixel>	tca(*maskimage);\
		fmask = new FourierImage(tca);				\
	}								\
}

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	bool	force = false;
	double	radius = 1;
	ImagePtr	maskptr;
	double	degree = 0.0;
	while (EOF != (c = getopt_long(argc, argv, "dhe:?fm:r:", longopts,
		&longindex))) {
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			degree = std::stod(optarg);
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
		case 'm':
			maskptr = io::FITSin(optarg).read();
			break;
		default:
			throw std::runtime_error("unknown option");
		}
	}

	// if the mask is not specified, throw an exception
	if (!maskptr) {
		std::runtime_error("mask must be specified, use option --mask");
	}

	// make sure we have at least 2 files
	if (optind >= argc) {
		std::cerr << "must specify image to get hdr" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	infile(argv[optind++]);
	if (optind >= argc) {
		std::cerr << "must specify output file name" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	outfile(argv[optind++]);

	// convert the mask image to double, as we have to fourier transform it
	FourierImage	*fmask = NULL;
	typeconvert(unsigned char, maskptr);
	typeconvert(unsigned short, maskptr);
	typeconvert(unsigned int, maskptr);
	typeconvert(unsigned long, maskptr);
	typeconvert(float, maskptr);
	typeconvert(double, maskptr);
	if (NULL == fmask) {
		std::runtime_error("cannot work with this mask type");
	}
	FourierImagePtr	fmaskptr(fmask);

	// get a gaussian blurring function
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create %s gauss with radius %f",
		maskptr->size().toString().c_str(), radius);
	TiledGaussImage	tg(maskptr->size(), radius, 1);
	Image<double>	*tgimage = new Image<double>(tg);
	ImagePtr	tgimageptr(tgimage);
	FourierImage	blurr(*tgimage);

	// convolve
	FourierImagePtr	blurred = (*fmask) * blurr;
	ImagePtr	blurredmaskptr = blurred->inverse();
	Image<double>	*blurredmask
		= dynamic_cast<Image<double>*>(&*blurredmaskptr);

	io::FITSout	blurredout("blurredout.fits");
	blurredout.setPrecious(false);
	blurredout.write(ImagePtr(tgimage));

	// open the input file
	io::FITSin	in(infile);
	ImagePtr	imageptr = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s-image of type %s", 
		imageptr->size().toString().c_str(),
		demangle(imageptr->pixel_type().name()).c_str());

	// hdr masking of image
	ImagePtr	outimage = do_deemphasize(imageptr, *blurredmask,
		degree);

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

} // namespace hdr
} // namespace app
} // namespace astro

int     main(int argc, char *argv[]) {
        return astro::main_function<astro::app::hdr::main>(argc, argv);
}

