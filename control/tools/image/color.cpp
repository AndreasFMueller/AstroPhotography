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

static RGB<double>	parse_color(const std::string& s) {
	// split the string at commas
	std::vector<std::string>	components;
	split<std::vector<std::string> >(s, ",", components);
	if (3 != components.size()) {
		std::string	msg = stringprintf("not a color spec: '%s'",
			s.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return RGB<double>(std::stod(components[0]),
			   std::stod(components[1]),
			   std::stod(components[2]));
}

class ColorBase {
protected:
	double	_gain;
	double	_base;
	double	_limit;
	RGB<double>	_scales;
	RGB<double>	_offsets;
public:
	double	gain() const { return _gain; }
	double	base() const { return _base; }
	double	limit() const { return _limit; }
	const RGB<double>&	scales() const { return _scales; }
	const RGB<double>&	offsets() const { return _offsets; }

	void	gain(double g) { _gain = g; }
	void	base(double b) { _base = b; }
	void	limit(double l) { _limit = l; }
	void	scales(const RGB<double>& s) { _scales = s; }
	void	offsets(const RGB<double>& o) { _offsets = o; }

	ColorBase() : _gain(1.0), _base(0.),
		_scales(1.), _offsets(0.) {
		_limit = -1;
	}
	ColorBase(const ColorBase& other) : _gain(other._gain),
		_base(other._base), _limit(other._limit),
		_scales(other._scales),
		_offsets(other._offsets) {
	}
};

template<typename T>
class ColorAdapter : public ConstImageAdapter<RGB<T> >, public ColorBase {
	const ConstImageAdapter<RGB<T> >&	_image;
public:
	ColorAdapter(const ConstImageAdapter<RGB<T> >& image)
		: ConstImageAdapter<RGB<T> >(image.getString()), _image(image) {
	}
	ColorAdapter(const ConstImageAdapter<RGB<T> >& image,
		const ColorBase& colorbase)
		: ConstImageAdapter<RGB<T> >(image.getSize()),
		  ColorBase(colorbase), _image(image) {
		if (_limit < 0) {
			_limit = std::numeric_limits<T>::max();
		}
	}
	virtual RGB<T>	pixel(int x, int y) const;
	static ImagePtr	color(const ConstImageAdapter<RGB<T> >& image,
				const ColorBase& colorsettings) {
		ColorAdapter<T>	ca(image, colorsettings);
		return ImagePtr(new Image<RGB<T> >(ca));
	}
};

template<typename T>
RGB<T>	ColorAdapter<T>::pixel(int x, int y) const {
	RGB<T>	v = _image.pixel(x, y);
	double	r = _gain * (_scales.R * v.R + _offsets.R) + _base;
	double	g = _gain * (_scales.G * v.G + _offsets.G) + _base;
	double	b = _gain * (_scales.B * v.B + _offsets.B) + _base;
	if (r < 0) { r = 0; }
	if (g < 0) { g = 0; }
	if (b < 0) { b = 0; }
	double	m = std::max(std::max(r, g), b);
	if (m > _limit) {
		r = _limit * r / m;
		g = _limit * g / m;
		b = _limit * b / m;
	}
	return RGB<T>(r, g, b);
}

#define	do_color(image, colorbase, Pixel)				\
	{								\
		Image<RGB<Pixel> >	*imagep				\
			= dynamic_cast<Image<RGB<Pixel> >*>(&*image);	\
		if (NULL != imagep) {					\
			return ColorAdapter<Pixel>::color(*imagep, colorbase);\
		}							\
	}

ImagePtr	color(ImagePtr image, const ColorBase& colorbase) {
	do_color(image, colorbase, unsigned char)
	do_color(image, colorbase, unsigned short)
	do_color(image, colorbase, unsigned int)
	do_color(image, colorbase, unsigned long)
	do_color(image, colorbase, float)
	do_color(image, colorbase, double)
	throw std::runtime_error("cannot change color for this pixel type");
}

int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	bool	force = false;
	ColorBase	colorbase;
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
			colorbase.scales(parse_color(optarg));
			break;
		case 'o':
			colorbase.offsets(parse_color(optarg));
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
	ImagePtr	outimage = color(image, colorbase);

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
