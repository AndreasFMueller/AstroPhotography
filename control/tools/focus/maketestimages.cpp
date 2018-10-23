/*
 * maketestimages.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <cmath>
#include <AstroUtils.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <getopt.h>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace app {
namespace maketestimages {

static float	sqr(float x) { return x * x; }

static float	rmin = -10, rmax = 10;

/**
 * \brief inverse of the error function
 */
double	ierf(double y) {
	double	y0 = y - 0.5;
	double	x = 0;
	double	delta = 100;
	do {
		double xnew = x - (erf(x) - y0)
					/ (exp(-x*x) / (2 / sqrt(M_PI)));
		delta = fabs(xnew - x);
		x = xnew;
	} while (delta > 0.001);
	return x;
}

/**
 * \brief A random number between 0 and 1
 */
double	frandom() {
	return random() / (double)RAND_MAX;
}

/**
 * \brief normally distributed random numbers
 */
double	normal(double mu = 0, double sigma = 1) {
	double	r = frandom();
	return sigma * ierf(r) + mu;
}

double	noise() {
	double v = normal(1, 8);
	if (v < 0) {
		v = 0;
	}
	return v;
}

static double	a = 1;

double	radius(double r) {
	return sqrt(a * a + r * r);
}

class star {
public:
	ImagePoint	point;
	float		brightness;
	star(int x, int y) : point(x,y), brightness(10) { }
	float	value(float r, float radius) {
		if (r <= radius) {
			return brightness;
		}
		return brightness * exp(-sqr(r - radius) / sqr(2));
	}
	float	value(const ImagePoint& p, float radius) {
		double	d = p.distance(point);
		return value(d, radius);
	}
	std::string	toString() const {
		return stringprintf("%s,%.3f", point.toString().c_str(),
			brightness);
	}
};

typedef std::list<star>	starlist_t;
static starlist_t	stars;

/**
 * \brief Construct a set of stars
 */
void	createstars(int n, const ImageSize& size) {
	stars.clear();
	for (int i = 0; i < n; i++) {
		int	x = random() % size.width();
		int	y = random() % size.height();
		star	s(x, y);
		s.brightness = 3 + 30 * frandom();
		stars.push_back(s);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "star: %s", s.toString().c_str());
	}
}

/**
 * \brief Create an image
 */
void	createimage(const std::string& filename, double radius,
		const ImageSize& size) {
	float	m = hypot(rmin, rmax) / (1 + radius);
	Image<float>	image(size);
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			image.pixel(x,y) = noise();
		}
	}
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			ImagePoint	p(x, y);
			float	s = image.pixel(x,y);
			for (auto i = stars.begin(); i != stars.end(); i++) {
				float	v = i->value(p, radius);
				s += v;
			}
			image.pixel(x,y) = m * s;
		}
	}
	io::FITSoutfile<float>	outfile(filename);
	outfile.write(image);
}

static struct option    longopts[] = {
{ "debug",	no_argument,		NULL,		'd' },
{ "minimum",	required_argument,	NULL,		'm' },
{ "maximum",	required_argument,	NULL,		'M' },
{ "number",	required_argument,	NULL,		'n' },
{ "stars",	required_argument,	NULL,		's' },
{ "prefix",	required_argument,	NULL,		'p' },
{ "width",	required_argument,	NULL,		'w' },
{ "height",	required_argument,	NULL,		'h' },
{ NULL,		0,			NULL,		 0  }
};


int	main(int argc, char *argv[]) {
	int	c;
	int	longindex;
	int	n = 10;
	int	h = 96;
	int	w = 128;
	int	s = 10;
	std::string	prefix("test");
        while (EOF != (c = getopt_long(argc, argv, "dm:M:n:s:p:h:w:", longopts,
		&longindex)))
                switch (c) {
                case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'm':
			rmin = atof(optarg);
			break;
		case 'M':
			rmax = atof(optarg);
			break;
		case 'n':
			n = atoi(optarg);
			break;
		case 's':
			s = atoi(optarg);
			break;
		case 'p':
			prefix = std::string(optarg);
			break;
		case 'h':
			h = atoi(optarg);
			break;
		case 'w':
			w = atoi(optarg);
			break;
		}

	createstars(s, ImageSize(w, h));

	float	delta = (rmax - rmin) / n;
	int	counter = 0;
	for (float r = rmin; r < rmax + delta/2; r += delta, counter++) {
		std::string	filename = stringprintf("%s-%03d.fits",
						prefix.c_str(), counter);
		createimage(filename, radius(r), ImageSize(w, h));
	}
	return EXIT_SUCCESS;
}

} // namespace maketestimages
} // namespace app
} // namespace astro

int     main(int argc, char *argv[]) {
        return astro::main_function<astro::app::maketestimages::main>(argc, argv);
}

