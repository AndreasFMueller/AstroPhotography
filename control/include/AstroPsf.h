/*
 * AstroPsf.h -- classes related to extracting a point spread function
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroImage.h>

namespace astro {
namespace psf {

/**
 * \brief Class to extract a point spread function for the luminance
 *
 * This class extracts a point spread function from a an image according
 * to the following algorithm
 * 1. find a set of isolated stars
 * 2. extract the luminance from the image for each of these points and
 *    build a local psf around that star
 * 3. for each star, find the centroid
 * 4. translate each star to 0 and add the stars
 * 5. normalize the psf so that the L^1-norm is 1
 */
class PsfExtractor {
	unsigned int	_radius;
public:
	unsigned int	radius() const { return _radius; }
	void	radius(unsigned int r) { _radius = r; }
private:
	unsigned int	_maxstars;
public:
	unsigned int	maxstars() const { return _maxstars; }
	void	maxstars(unsigned int m) { _maxstars = m; }
public:
	PsfExtractor();
	image::Image<double>	*extract(image::ImagePtr image);
};

} // namespace psf
} // namespace astro
