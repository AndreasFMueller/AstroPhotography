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
	std::cout << "    " << path.basename() << " [ options ] <filename>" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -a,--altaz                  toggle display of altitude and azimut" << std::endl;
	std::cout << "  -c,--constellations         toggle the display of constellations" << std::endl;
	std::cout << "  -C,--cardinal               toggle labels for cardinal directions" << std::endl;
	std::cout << "  --copyright                 toggle display copyright string" << std::endl;
	std::cout << "  -d,--debug                  increase debug level" << std::endl;
	std::cout << "  -D,--declination=<dec>      DEC of the telescope marker" << std::endl;
	std::cout << "  -e,--ecliptic               toggle display of the ecliptic" << std::endl;
	std::cout << "  -g,--grid                   toggle the RA/DEC grid" << std::endl;
	std::cout << "  -h,-?,--help                show this help message and exit" << std::endl;
	std::cout << "  -L,--longitude=<long>       longitude of the observatory" << std::endl;
	std::cout << "  -l,--latitude=<lat>         latitude of the observatory" << std::endl;
	std::cout << "  -m,--milkyway               toggle milkyway display" << std::endl;
	std::cout << "  -p,--position               toggle display the position" << std::endl;
	std::cout << "  -P,--pole                   toggle showing the pole" << std::endl;
	std::cout << "  -R,--rightascension=<ra>    RA of the telescope marker" << std::endl;
	std::cout << "  -s,--size=<s>               generate a <s>x<s> image, default is 1024" << std::endl;
	std::cout << "  -S,--timestamp              toggle display of a timestamp" << std::endl;
	std::cout << "  -t,--time=<t>               time for which to draw the image" << std::endl;
	std::cout << "  -T,--telescope-coord        toggle printing the telescope coordinates" << std::endl;
	std::cout << "  -X,--target-coord           toggle printing the target coordinates" << std::endl;
	std::cout << "  -Y,--target-ra=<ra>         right ascension of the target" << std::endl;
	std::cout << "  -Z,--target-dec=<dec>       declination of the target" << std::endl;
	std::cout << "  -v,--verbose                verbose display" << std::endl;
	std::cout << std::endl;
}

static struct option	longopts[] = {
{ "altaz",		no_argument,		NULL,		'a' },
{ "constellations",	no_argument,		NULL,		'c' },
{ "cardinal",		no_argument,		NULL,		'C' },
{ "copyright",		no_argument,		NULL,		'f' },
{ "debug",		no_argument,		NULL,		'd' },
{ "declination",	required_argument,	NULL,		'D' },
{ "ecliptic",		no_argument,		NULL,		'e' },
{ "grid",		no_argument,		NULL,		'g' },
{ "rightascension",	required_argument,	NULL,		'R' },
{ "help",		no_argument,		NULL,		'h' },
{ "longitude",		required_argument,	NULL,		'L' },
{ "latitude",		required_argument,	NULL,		'l' },
{ "milkyway",		no_argument,		NULL,		'm' },
{ "position",		no_argument,		NULL,		'p' },
{ "pole",		no_argument,		NULL,		'P' },
{ "size",		required_argument,	NULL,		's' },
{ "timestamp",		no_argument,		NULL,		'S' },
{ "time",		required_argument,	NULL,		't' },
{ "telescope-coord",	no_argument,		NULL,		'T' },
{ "target-coord",	no_argument,		NULL,		'X' },
{ "target-ra",		required_argument,	NULL,		'Y' },
{ "target-dec",		required_argument,	NULL,		'Z' },
{ "verbose",		no_argument,		NULL,		'v' },
{ NULL,			0,			NULL,		 0  }
};

#define	YESNO(b) ((b) ? "yes" : "no")

int	main(int argc, char *argv[]) {
	debug_set_ident("astrosky");
	debugthreads = 1;
	bool	verbose = false;

	astro::LongLat	position;
	astro::RaDec	telescope;
	astro::RaDec	target;
	SkyDrawing	skydrawing;

	int	c;
	int	longindex;
	int	s = 1024;
	time_t	t = 0;
	while (EOF != (c = getopt_long(argc, argv,
		"acdefgh?L:l:mpPs:SD:R:t:vTXY:Z:", longopts, &longindex)))
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
		case 'f':
			skydrawing.show_copyright(!skydrawing.show_copyright());
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
			return EXIT_SUCCESS;
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
		case 'm':
			skydrawing.show_milkyway(!skydrawing.show_milkyway());
			break;
		case 'D':
			telescope.dec() = astro::Angle(std::stod(optarg),
				astro::Angle::Degrees);
			skydrawing.telescopeChanged(telescope);
			skydrawing.show_telescope(true);
			break;
		case 'p':
			skydrawing.show_position(!skydrawing.show_position());
			break;
		case 'P':
			skydrawing.show_pole(!skydrawing.show_pole());
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
		case 'S':
			skydrawing.show_time(!skydrawing.show_time());
			break;
		case 't':
			t = std::stoi(optarg);
			break;
		case 'T':
			skydrawing.show_telescope_coord(
				!skydrawing.show_telescope_coord());
			break;
		case 'X':
			skydrawing.show_target_coord(
				!skydrawing.show_target_coord());
			break;
		case 'Y':
			target.ra() = astro::Angle(std::stod(optarg),
				astro::Angle::Hours);
			skydrawing.targetChanged(target);
			skydrawing.show_target(true);
			break;
		case 'Z':
			target.dec() = astro::Angle(std::stod(optarg),
				astro::Angle::Degrees);
			skydrawing.targetChanged(target);
			skydrawing.show_target(true);
			break;
		case 'v':
			verbose = true;
			break;
		}

	QApplication	*app = NULL;
	if (	skydrawing.show_labels() ||
		skydrawing.show_constellation_labels() ||
		skydrawing.show_telescope_coord() ||
		skydrawing.show_target_coord() ||
		skydrawing.show_pole() ||
		skydrawing.show_copyright() ||
		skydrawing.show_position() ||
		skydrawing.show_time()) {
		try {
			app = new QApplication(argc, argv);
		} catch (const std::exception& x) {
			debug(LOG_ERR, DEBUG_LOG, 0,
				"cannot create application: %s", x.what());
		}
	}
	if (NULL == app) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot draw any text");
		skydrawing.show_labels(false);
		skydrawing.show_constellation_labels(false);
		skydrawing.show_telescope_coord(false);
		skydrawing.show_target_coord(false);
		skydrawing.show_pole(false);
		skydrawing.show_copyright(false);
		skydrawing.show_position(false);
		skydrawing.show_time(false);
	}

	// next argument must be a filename
	if (optind >= argc) {
		std::cerr << "file name missing" << std::endl;
		return EXIT_FAILURE;
	}
	std::string	filename(argv[optind++]);

	// create the drawing object
	skydrawing.positionChanged(position);
	if (t) {
		skydrawing.time(t);
	}

	// get the star catalog
	astro::catalog::CatalogPtr catalog
		= astro::catalog::CatalogFactory::get();
	astro::catalog::SkyWindow       windowall;
	astro::catalog::MagnitudeRange  magrange(-30, 6);
	astro::catalog::Catalog::starsetptr     stars
		= catalog->find(windowall, magrange);
	astro::Precession       precession;
	stars = precess(precession, stars);
	skydrawing.useStars(stars);

	// display information about what we are doing
	if (verbose) {
		std::cout << "Location:            "
			<< position.toString() << std::endl;
		std::cout << "Cardinal directions: "
			<< YESNO(skydrawing.show_labels()) << std::endl;
		std::cout << "RA/DEC grid:         "
			<< YESNO(skydrawing.show_radec()) << std::endl;
		std::cout << "constellations:      "
			<< YESNO(skydrawing.show_constellations()) << std::endl;
		std::cout << "constellation labels:"
			<< YESNO(skydrawing.show_constellation_labels()) << std::endl;
		std::cout << "Poles:               "
			<< YESNO(skydrawing.show_pole()) << std::endl;
		std::cout << "Ecliptic:            "
			<< YESNO(skydrawing.show_ecliptic()) << std::endl;
		std::cout << "Milkyway:            "
			<< YESNO(skydrawing.show_milkyway()) << std::endl;
		std::cout << "Telescope:           "
			<< YESNO(skydrawing.show_telescope()) << std::endl;
		std::cout << "Telescope coords:    "
			<< telescope.toString() << std::endl;
		std::cout << "Target:              "
			<< YESNO(skydrawing.show_target()) << std::endl;
		std::cout << "Target coords:       "
			<< target.toString() << std::endl;
		std::cout << "Position:            "
			<< YESNO(skydrawing.show_position()) << std::endl;
		std::cout << "Time:                "
			<< YESNO(skydrawing.show_time()) << std::endl;
		std::cout << "Copyright:           "
			<< YESNO(skydrawing.show_copyright()) << std::endl;
	}

	// create a QImage 
	QSize	size(s, s);
	QImage	image(size, QImage::Format_ARGB32);
	QPainter	painter(&image);
	//painter.fillRect(0, 0, s, s, Qt::black);
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
