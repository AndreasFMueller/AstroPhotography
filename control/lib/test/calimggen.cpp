/*
 * calimggen.cpp -- generate randomized images that can be used to test
 *                  the image calibration stuff
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <calimggen.h>

namespace astro {
namespace calibration {
namespace test {

void	CalImgGen::configure() {
	if (!changed) {
		return;
	}

	// XXX rebuild the image
}

CalImgGen::CalImgGen(unsigned int _width, unsigned int _height)
	: width(_width), height(_height) {
}

double	CalImgGen::getNoise() const {
	return noise;
}

void	CalImgGen::setNoise(const double _noise) {
	noise = _noise;
	changed = true;
}

double	CalImgGen::getDarklevel() const {
	return darklevel;
}

void	CalImgGen::setDarklevel(const double _darklevel) {
	darklevel = _darklevel;
	changed = true;
}

double	CalImgGen::getDarkvariance() const {
	return darkvariance;
}

void	CalImgGen::setDarkvariance(const double _darkvariance) {
	darkvariance = _darkvariance;
	changed = true;
}

double	CalImgGen::getFlatlevel() const {
	return flatlevel;
}

void	CalImgGen::setFlatlevel(const double _flatlevel) {
	flatlevel = _flatlevel;
	changed = true;
}

double	CalImgGen::getFlatcurvature() const {
	return flatcurvature;
}

void	CalImgGen::setFlatcurvature(const double _flatcurvature) {
	flatcurvature = _flatcurvature;
	changed = true;
}

double	CalImgGen::getBadpixel() const {
	return badpixel;
}

void	CalImgGen::setBadpixel(const double _badpixel) {
	badpixel = _badpixel;
	changed = true;
}

double	CalImgGen::getBadcolumn() const {
	return badcolumn;
}

void	CalImgGen::setBadcolumn(const double _badcolumn) {
	badcolumn = _badcolumn;
	changed = true;
}

int	main(int argc, char *argv[]) {
	return EXIT_SUCCESS;
}

} // namespace test
} // namespace calibration
} // namespace astro

int	main(int argc, char *argv[]) {
	return astro::calibration::test::main(argc, argv);
}
