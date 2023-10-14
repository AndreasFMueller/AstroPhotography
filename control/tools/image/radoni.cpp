/*
 * radoni.cpp -- radon transform of an image
 *
 * (c) 2023 Prof Dr Andreas Müller
 */
#include <includes.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <Radon.h>
#include <AstroAdapter.h>
#include <fftw3.h>

using namespace astro;
using namespace astro::io;
using namespace astro::image;
using namespace astro::image::radon;
using namespace astro::adapter;

namespace astro {
namespace app {
namespace radoni {


static double	radius = 1.0;
static bool	logvalues = false;

/**
 * \brief convert the value to displayable value
 *
 * \param x	value to represent
 */
static double	displayvalue(double x) {
	if (logvalues) {
		if (x < 1) {
			return 0;
		} else {
			return log(x);
		}
	} else {
		return x;
	}
}

class FourierImage;

/**
 * \brief one dimensional Fourier image
 */
class FourierLine {
	int	_width;
	fftw_complex	*_data;
	FourierLine(const FourierLine& other) = delete;
	FourierLine&	operator=(const FourierLine& other) = delete;
	void	setup(bool initialize) {
		_data = (fftw_complex *)fftw_malloc(
			sizeof(fftw_complex) * _width);
		if (initialize) {
			for (int x = 0; x < _width; x++) {
				_data[x][0] = 0;
				_data[x][1] = 0;
			}
		}
	}
public:
	int	width() const { return _width; }
	const fftw_complex	*data() const { return _data; }
	fftw_complex	*data() { return _data; }
	FourierLine(int width) : _width(width) {
		setup(true);
	}
	~FourierLine() {
		fftw_free(_data);
		_data = NULL;
	}
	const fftw_complex&	pixel(int i) const { return _data[i]; }
	fftw_complex&	pixel(int i) { return _data[i]; }
	fftw_complex&	pixel(int i, const fftw_complex& c) {
		_data[i][0] = c[0];
		_data[i][1] = c[1];
		return _data[i];
	}
	FourierLine(const FourierImage& fi, int y);
	FourierLine(const FourierLine& fl, int direction)
		: _width(fl.width()) {
		setup(false);
		fftw_plan	p = fftw_plan_dft_1d(width(),
					const_cast<fftw_complex*>(fl.data()),
					_data, direction, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);
	}
};

/**
 * \brief Class encapsulating the fourier image
 */
class FourierImage {
	ImageSize	_size;
	fftw_complex	*_data;
	int	offset(int x, int y) const {
		return y + _size.height() * x;
	}
	void	setup(bool initialize) {
		_data = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * 
			_size.width() * _size.height());
		if (initialize) {
			int	l = _size.getPixels();
debug(LOG_DEBUG, DEBUG_LOG, 0, "initializing pxels: %d", l);
			for (int i = 0; i < l; i++) {
				_data[i][0] = 0;
				_data[i][1] = 0;
			}
		}
	}
public:
	const ImageSize&	size() const { return _size; }
	const fftw_complex	*data() const { return _data; }
	fftw_complex	*data() { return _data; }
	FourierImage(const ImageSize& size) : _size(size) {
		setup(true);
	}
	FourierImage(ImagePtr imageptr, const ImagePoint& offset);
	~FourierImage() {
		fftw_free(_data);
		_data = NULL;
	}
private:
	FourierImage(const FourierImage& other) = delete;
	FourierImage&	operator=(const FourierImage& other) = delete;
public:
	const fftw_complex&	pixel(int x, int y) const {
		return _data[offset(x, y)];
	}
	fftw_complex&	pixel(int x, int y) {
		return _data[offset(x, y)];
	}
	const fftw_complex&	pixel(int x, int y,
		const fftw_complex& v) const {
		int	i = offset(x, y);
		_data[i][0] = v[0];
		_data[i][1] = v[1];
		return _data[i];
	}
	fftw_complex&	pixel(int x, int y, const fftw_complex& v) {
		int	i = offset(x, y);
		_data[i][0] = v[0];
		_data[i][1] = v[1];
		return _data[i];
	}
	fftw_complex&	pixel(int x, int y, double v) {
		int	i = offset(x, y);
		_data[i][0] = v;
		_data[i][1] = 0;
		return _data[i];
	}
	void	pixels(int y, const FourierLine& fl) {
		int	w = _size.width();
		if (fl.width() < w) {
			w = fl.width();
		}
		for (int x = 0; x < w; x++) {
			pixel(x, y, fl.pixel(x));
		}
	}
	double	abspixel(int x, int y) const {
		return hypot(pixel(x, y)[0], pixel(x, y)[1]);
	}
	double	phipixel(int x, int y) const {
		return atan2(pixel(x, y)[1], pixel(x, y)[0]);
	}
	ImagePtr	image(const ImagePoint& offset, bool log);
	ImagePtr	imagergb(const ImagePoint& offset, bool log);
	ImagePtr	write(const std::string& filename,
				const ImagePoint& offset, bool log);
	ImagePtr	writergb(const std::string& filename,
				const ImagePoint& offset, bool log);
	const fftw_complex&	polarpixel(int r, int phi) const {
		double	ph = phi * 2 * M_PI / size().height();
		int 	x = round(r * cos(ph));
		int	y = round(r * sin(ph));
		if (x >= 0) {
			if (y >= 0) {
				return pixel(x, y);
			} else {
				return pixel(x, size().height() - y - 1);
			}
		} else {
			if (y >= 0) {
				return pixel(size().width() - x - 1, y);
			} else {
				return pixel(size().width() - x - 1,
					size().height() - y - 1);
			}
		}
	}
private:
	void	f1topolar(const FourierImage& f1);
	void	f1frompolar(const FourierImage& f2) {
	}
public:
	FourierImage(const FourierImage& f, bool topolar) : _size(f.size()) {
		setup(true);
		if (topolar) {
			f1topolar(f);
		} else {
			f1frompolar(f);
		}
	}
	FourierImage(const FourierImage& f, int direction) : _size(f.size()) {
		setup(false);
		fftw_plan	p = fftw_plan_dft_2d(
					_size.width(), _size.height(),
					const_cast<fftw_complex*>(f.data()),
					_data, direction, FFTW_ESTIMATE);
		fftw_execute(p);
		fftw_destroy_plan(p);
	}
	void	maskcircle(double radius) {
		int	w = _size.width();
		int	h = _size.height();
		int	m = (w > h) ? w : h;
		double	r = radius * (m/2);
		for (int x = 0; x < w; x++) {
			for (int y = 0; y < h; y++) {
				double	d00 = hypot(x, y);
				double	d01 = hypot(x, y - h);
				double	d10 = hypot(x - w, y);
				double	d11 = hypot(x - w, y - h);
				if ((d00 > r) && (d01 > r) && (d10 > r) && (d11 > r)) {
					pixel(x, y, 0.);
				}
			}
		}
	}
};

FourierLine::FourierLine(const FourierImage& fi, int y)
	: _width(fi.size().width()) {
	setup(false);
	for (int x = 0; x < _width; x++) {
		pixel(x, fi.pixel(x, y));
	}
}

FourierImage::FourierImage(ImagePtr imageptr, const ImagePoint& offset)
		: _size(imageptr->size()) {
	ConstPixelValueAdapter<double>	image(imageptr);
	int	w = imageptr->size().width();
	int	h = imageptr->size().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image dimensions: %d x %d, offset = %s",
		w, h, offset.toString().c_str());

	// prepare the input data for the 2D Fourier transform
	setup(false);
	for (int x = 0; x < w; x++) {
		int	X = x - offset.x();
		if (X < 0) { X += w; }
		for (int y = 0; y < h; y++) {
			int	Y = y - offset.y();
			if (Y < 0) { Y += h; }
			double	v = image.pixel(X, Y);
			pixel(x, y, v);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "pixel values copied");
}

/**
 * \brief display a message about the radoni program
 */
static void	usage(const char *progname) {
	Path	p(progname);
	std::cout << "usage:" << std::endl;
	std::cout << "    " << p.basename() << " [ options ] infile outfile"
		<< std::endl;
	std::cout << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "   -d,--debug       increase debug level" << std::endl;
	std::cout << "   -r,--radius=<r>  set masking radius between 0 and 1"
		<< std::endl;
	std::cout << "   -l,--log         use logarithm for value display"
		<< std::endl;
	std::cout << std::endl;
}

/**
 * \brief convert an FFT image to a Astro image
 *
 * \param imagedata	the fftw_complex array of image data
 * \param size		the size of the image
 * \param offset	the offset to use when converting (used to get 0 into
 *			the middle of the frame)
 * \param log		whether or not to use the displayvalue function
 *			when converting pixel values
 */
ImagePtr	FourierImage::image(const ImagePoint& offset, bool log) {
	int	w = size().width();
	int	h = size().height();
	Image<double>	*output = new Image<double>(size());
	ImagePtr	outputptr(output);
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			double	v = abspixel(x, y);
			if (log) v = displayvalue(v);
			output->pixel((x + offset.x()) % w,
				(y + offset.y()) % h) = v;
		}
	}
	return outputptr;
}

ImagePtr	FourierImage::imagergb(const ImagePoint& offset, bool log) {
	int	w = size().width();
	int	h = size().height();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing RGB image");
	Image<RGB<double> >	*output = new Image<RGB<double> >(size());
	ImagePtr	outputptr(output);
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			double	v = abspixel(x, y);
			double	phi = phipixel(x, y);
			if (phi < 0) {
				phi += 2 * M_PI;
			}
			if (log) v = displayvalue(v);
			HSV<double>	hsv(phi, 1, v);
			RGB<double>	rgb(hsv.r(), hsv.g(), hsv.b());
			output->pixel((x + offset.x()) % w,
				(y + offset.y()) % h) = rgb;
		}
	}
	return outputptr;
}

/**
 * \param Write an 
 *
 * \param filename	the name of the fits file to write
 * \param imagedata	the fftw_complex image data
 * \param size		the size of the image
 * \param offset	the offset to use to get the 0 frequency into the
 *			middle of the frame
 * \param log		whether or not to use the 
 */
ImagePtr	FourierImage::write(const std::string& filename,
				const ImagePoint& offset, bool log) {
	ImagePtr	outputptr = image(offset, log);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing %s",
		filename.c_str());
	unlink(filename.c_str());
	FITSout	out(filename);
	out.setPrecious(false);
	out.write(outputptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image written");
	return outputptr;
}

ImagePtr	FourierImage::writergb(const std::string& filename,
			const ImagePoint& offset, bool log) {
	ImagePtr	outputptr = imagergb(offset, log);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing rgb image %s",
		filename.c_str());
	unlink(filename.c_str());
	FITSout	out(filename);
	out.setPrecious(false);
	out.write(outputptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rgb image written");
	return outputptr;
}

/**
 * \brief convert to polar
 */
void	FourierImage::f1topolar(const FourierImage& f1) {
	int	w = size().width();
	int	h = size().height();
	for (int x = -w/2 + 1; x < w/2; x++) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "f1tpolor x = %d", x);
		for (int y = 0; y < h/2; y++) {
			int	ri = round(hypot(x, y));
			if (ri > radius * w)
				continue;
			double	phi = atan2(y, x);
			if (phi < 0) {
				phi = phi + 2 * M_PI;
			}
			int	phii = round(h * phi /  M_PI);
			phii = phii % h;
			if (x >= 0) {
				pixel(x, y, f1.pixel(ri, phii));
				pixel(w - x - 1, h - y - 1, f1.pixel(w - 1 - ri, phii));
			}
			if (x < 0) {
				pixel(x + w, y, f1.pixel(ri, phii));
				pixel(-x, h - y - 1, f1.pixel(w - ri, phii));
			}
		}
	}
}
/**
 * \brief backward computation from final image
 *
 * \param bild		base name of the image
 */
static void	backward(const std::string& bild) {
	std::string	fromname = bild + std::string(".fits");

	std::string	prefix = bild + std::string("-backward-");
	std::string	fftname = prefix + std::string("FT.fits");
	std::string	fftlname = prefix + std::string("FTL.fits");
	std::string	polarfftname = prefix + std::string("FT-polar.fits");
	std::string	polarfftlname = prefix + std::string("FTL-polar.fits");
	std::string	radonname = prefix + std::string("radon.fits");
	std::string	maskedfftname = prefix + std::string("FT-masked.fits");
	std::string	maskedfftlname = prefix + std::string("FTL-masked.fits");
	std::string	maskedname = prefix + std::string("masked.fits");

	// read the image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading image %s", fromname.c_str());
	FITSin	in(fromname);
	ImagePtr	imageptr = in.read();
	int	w = imageptr->size().width();
	int	h = imageptr->size().height();

	// convert to complex fft image
	FourierImage	space(imageptr, imageptr->size().center());

	// perform the 2D Fourier transform of the image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transforming input image");
	FourierImage	frequency(space, -1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "input image transformed");
	frequency.writergb(fftname, imageptr->size().center(), true);
	frequency.write(fftlname, imageptr->size().center(), true);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transformed image written to %s",
		fftname.c_str());

	// convert to polar
	FourierImage	polar(imageptr->size());
	for (int phii = 0; phii < h; phii++) {
		for (int ri = -w/2 + 1; ri < w/2; ri++) {
			double	phi = phii * M_PI / h;
			int	x = round(ri * cos(phi));
			if (x < 0) { x = x + w; }
			int	y = round(ri * sin(phi));
			if (y < 0) { y = y + h; }
			if (ri < 0) {
				polar.pixel(w + ri, phii,
					frequency.pixel(x, y));
			} else {
				polar.pixel(ri, phii, frequency.pixel(x, y));
			}
		}
	}
	polar.writergb(polarfftname, ImagePoint(w/2, 0), true);
	polar.write(polarfftlname, ImagePoint(w/2, 0), true);

	// perform a fourier transform on every line
	FourierImage	radon(polar.size());
	for (int phii = 0; phii < h; phii++) {
		FourierLine	line(polar, phii);
		FourierLine	fline(line, 1);
		radon.pixels(phii, fline);
	}
	radon.write(radonname, ImagePoint(w/2, 0), false);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "backward radon written to %s",
		radonname.c_str());

	// mask out a circle
	debug(LOG_DEBUG, DEBUG_LOG, 0, "masking circle");
	frequency.maskcircle(radius);
	frequency.writergb(maskedfftname, imageptr->size().center(), true);
	frequency.write(maskedfftlname, imageptr->size().center(), true);

	// transform back
	debug(LOG_DEBUG, DEBUG_LOG, 0, "back transformation");
	FourierImage	maskedimage(frequency, 1);

	// write out the transformed image
	maskedimage.write(maskedname, imageptr->size().center(), false);
}

/**
 * \brief forward computation
 *
 * \param bild		base name of the imge
 */
static void	forward(const std::string& bild) {
	std::string	infilename = bild + "-radon.fits";
	std::string	f1filename = bild + "-forward-F1.fits";
	std::string	f2filename = bild + "-forward-F2.fits";
	std::string	ofilename = bild + "-forward-o.fits";

	// read the input image
	FITSin	in(infilename);
	ImagePtr	imageptr = in.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an image of dimension %d x %d",
		imageptr->size().width(), imageptr->size().height());

	// compute horizontal fourier transforms for every line
	int	h = imageptr->size().height();
	int	w = imageptr->size().width();
	FourierImage	inputimage(imageptr, ImagePoint(w/2, 0));

	// compute fourier transform on each row
	FourierImage	f1image(imageptr->size());
	for (int y = 0; y < h; y++) {
		//debug(LOG_DEBUG, DEBUG_LOG, 0, "FFT for line %d", y);
		FourierLine	fl(inputimage, y);
		FourierLine	flfft(fl, -1);
		f1image.pixels(y, flfft);
	}
	f1image.writergb(f1filename, ImagePoint(w/2, 0), true);

	// convert the image to polar coordinates
	FourierImage	f2image(f1image, true);
	f2image.writergb(f2filename, f2image.size().center(), true);

	// 
}

static struct option	longopts[] = {
{ "debug",		no_argument,		NULL,		'd' },
{ "radius",		required_argument,	NULL,		'r' },
{ "log",		no_argument,		NULL,		'l' },
{ NULL,			0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	while (EOF != (c = getopt_long(argc, argv, "d?r:l",
		longopts, &longindex)))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'r':
			radius = std::stod(optarg);
			break;
		case 'l':
			logvalues = true;
			break;
		}

	// next argument must be the file names
	if ((argc - optind) != 1) {
		std::cerr << "wrong number of arguments" << std::endl;
		usage(argv[0]);
		return EXIT_FAILURE;
	}
	std::string	bild = argv[optind++];

	// produce the backware images
	backward(bild);

	// do the forward computation
	forward(bild);

	return EXIT_SUCCESS;
}

} // namespace radoni
} // namespace app
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::main_function<astro::app::radoni::main>(argc, argv);
}


