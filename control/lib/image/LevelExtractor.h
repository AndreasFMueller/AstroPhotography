/*
 * LevelExtractor.h -- class to extract stars from an image
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <AstroFilter.h>

namespace astro {
namespace image {
namespace transform {

/**
 * \brief Level Extractor class
 *
 * This class analyzes an image an constructs a set of stars for the image.
 * A a star is an isolated maximum of the luminance of the image. Some
 * parameters control how stars are selected.
 */
class LevelExtractor {
	int	_maxstars;
public:
	int	maxstars() const { return _maxstars; }
	void	maxstars(int m) { _maxstars = m; }
private:
	double	_level;
public:
	double	level() const { return _level; }
	void	level(double l) { _level = l; }
private:
	/**
	 * \brief Search radius for stars
	 *
	 * The radius ensures that stars are the brightest points within a
	 * given radius.
	 */
	double	_radius;
public:
	double	radius() const { return _radius; }
	void	radius(double r) { _radius = r; }
private:
	std::set<Star>	_stars;

	int	close(int x, int y) const;

	int	inspectpoint(const ConstImageAdapter<double>& image,
			int x, int y, double limit,
			const StarAcceptanceCriterion& criterion);
public:

	const std::set<Star>&	stars() const { return _stars; }
	size_t	nstars() const { return _stars.size(); }
	std::vector<Star>	stars(unsigned int n);

	LevelExtractor(double level);
	void	analyze(const ConstImageAdapter<double>& image,
			const StarAcceptanceCriterion& criterion);
};

} // namespace transform
} // namespace image
} // namespace astro
