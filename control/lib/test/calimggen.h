/*
 * calimggen.h -- generate calibration images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>

namespace astro {
namespace calibration {
namespace test {

class CalImgGen {
	// note changes so that we can regenerate everything if needed
	bool	changed;
	void	configure();

	// (unmutable) dimensions of image to retrieve
	const unsigned int	width;
	const unsigned int	height;

	// noise variance present in all types of images
	double	noise;

	// parameters for dark levels
	double	darklevel;
	double	darkvariance;

	// parameters for flat images
	double	flatlevel;
	double	flatcurvature;
	double	flatvariance;

	// parameters for bad pixels
	double	badpixel;
	double	badcolumn;

	// vectors of bad pixels and bad columns
	typedef std::pair<unsigned int, unsigned int>	point;
	std::vector<point>	badpixels;
	std::vector<point>	badcolumns;
	
public:
	CalImgGen(unsigned int width, unsigned int height);

	double	getNoise() const;
	void	setNoise(const double noise);

	double	getDarklevel() const;
	void	setDarklevel(const double darklevel);

	double	getDarkvariance() const;
	void	setDarkvariance(const double darkvariance);

	double	getFlatlevel() const;
	void	setFlatlevel(const double flatlevel);

	double	getFlatcurvature() const;
	void	setFlatcurvature(const double flatcurvature);

	double	getBadpixel() const;
	void	setBadpixel(const double badpixel);

	double	getBadcolumn() const;
	void	setBadcolumn(const double badcolumn);

	astro::image::ImagePtr	get();
};

} // namespace test
} // namespace calibration
} // namespace astro
