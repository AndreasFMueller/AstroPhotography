/*
 * astrosky.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <AstroCoordinates.h>
#include <getopt.h>
#include <SkyDrawing.h>
#include <QImage>
#include <QPainter>
#include <QApplication>

namespace snowgui {
namespace sky {

void	usage(char *progname) {
	astro::Path	path(progname);
	std::cout << "Usage:" << std::endl;
	std::cout << "    " << path.basename() << " [ options ]" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -a,--altaz                  toggle display of altitude and azimut" << std::endl;
	std::cout << "  -c,--constellations         toggle the display of constellations" << std::endl;
	std::cout << "  -C,--cardinal               toggle labels for cardinal directions" << std::endl;
	std::cout << "  -d,--debug                  increase debug level" << std::endl;
	std::cout << "  -D,--declination=<dec>      DEC of the telescope marker" << std::endl;
	std::cout << "  -e,--ecliptic               toggle display of the ecliptic" << std::endl;
	std::cout << "  -g,--grid                   toggle the RA/DEC grid" << std::endl;
	std::cout << "  -h,-?,--help                show this help message and exit";
	std::cout << "  -L,--longitude<long>        longitude of the observatory" << std::endl;
	std::cout << "  -l,--latitude<lat>          latitude of the observatory" << std::endl;
	std::cout << "  -R,--rightascension=<ra>    RA of the telescope marker" << std::endl;
	std::cout << "  -s,--size=<s>               generate a <s>x<s> image, default is 1024" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "altaz",		no_argument,		NULL,		'a' },
{ "constellations",	no_argument,		NULL,		'c' },
{ "cardinal",		no_argument,		NULL,		'C' },
{ "debug",		no_argument,		NULL,		'd' },
{ "declination",	required_argument,	NULL,		'D' },
{ "ecliptic",		no_argument,		NULL,		'e' },
{ "grid",		no_argument,		NULL,		'g' },
{ "rightascension",	required_argument,	NULL,		'R' },
{ "help",		no_argument,		NULL,		'h' },
{ "longitude",		required_argument,	NULL,		'L' },
{ "latitude",		required_argument,	NULL,		'l' },
{ "size",		required_argument,	NULL,		's' },
{ NULL,			0,			NULL,		 0  }
};

int	main(int argc, char *argv[]) {
	debug_set_ident("astrosky");
	debugthreads = 1;

	astro::LongLat	position;
	astro::RaDec	telescope;
	SkyDrawing	skydrawing;


	int	c;
	int	longindex;
	int	s = 1024;
	while (EOF != (c = getopt_long(argc, argv, "acdegh?L:l:s:D:R:",
		longopts, &longindex)))
		switch (c) {
		case 'a':
			skydrawing.show_altaz(
				!skydrawing.show_altaz());
			break;
		case 'c':
			skydrawing.show_constellations(
				!skydrawing.show_constellations());
			break;
		case 'C':
			skydrawing.show_labels(!skydrawing.show_labels());
			break;
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'g':
			skydrawing.show_radec(
				!skydrawing.show_radec());
			break;
		case 'e':
			skydrawing.show_ecliptic(
				!skydrawing.show_ecliptic());
			break;
		case 'h':
		case '?':
			usage(argv[0]);
			break;
		case 'L':
			position.longitude() = astro::Angle(std::stod(optarg),
				astro::Angle::Degrees);
			skydrawing.positionChanged(position);
			break;
		case 'l':
			position.latitude() = astro::Angle(std::stod(optarg),
				astro::Angle::Degrees);
			skydrawing.positionChanged(position);
			break;
		case 'D':
			telescope.dec() = astro::Angle(std::stod(optarg),
				astro::Angle::Degrees);
			skydrawing.telescopeChanged(telescope);
			skydrawing.show_telescope(true);
			break;
		case 'R':
			telescope.ra() = astro::Angle(std::stod(optarg),
				astro::Angle::Hours);
			skydrawing.telescopeChanged(telescope);
			skydrawing.show_telescope(true);
			break;
		case 's':
			s = std::stoi(optarg);
			break;
		}

	QApplicaton	*app = NULL;
	if (skydrawing.show_labels()) {
		app = new QApplication(argc, argv);
	}

	// next argument must be a filename
	if (optind >= argc) {
		std::cerr << "file name missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	filename(argv[optind++]);

	// create the drawing object
	skydrawing.positionChanged(position);

	// get the star catalog
	astro::catalog::CatalogPtr catalog = astro::catalog::CatalogFactory::get();
	astro::catalog::SkyWindow       windowall;
	astro::catalog::MagnitudeRange  magrange(-30, 6);
	astro::catalog::Catalog::starsetptr     stars
		= catalog->find(windowall, magrange);
	astro::Precession       precession;
	stars = precess(precession, stars);
	skydrawing.useStars(stars);

	// create a QImage 
	QSize	size(s, s);
	QImage	image(size, QImage::Format_ARGB32);
	QPainter	painter(&image);
	QColor	transparent(0, 0, 0, 0);
	painter.fillRect(0, 0, s, s, transparent);

	// draw the star chart
	skydrawing.draw(painter, size);

	// write the QImage to a PNG file
	image.save(filename.c_str());

	return EXIT_SUCCESS;
}

} // namespace sky
} // namespace snowgui

int	main(int argc, char *argv[]) {
	return astro::main_function<snowgui::sky::main>(argc, argv);
}
