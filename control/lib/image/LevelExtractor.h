/*
 * LevelExtractor.cpp -- class to extract stars from an image
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
	double	_radius;
public:
	double	radius() const { return _radius; }
	void	radius(double r) { _radius = r; }
private:
	std::set<Star>	_stars;

	int	inspectpoint(const ConstImageAdapter<double>& image,
			int x, int y);
public:

	const std::set<Star>&	stars() const { return _stars; }
	size_t	nstars() const { return _stars.size(); }
	std::vector<Star>	stars(int n);

	LevelExtractor(double level);
	void	analyze(const ConstImageAdapter<double>& image);
};

} // namespace transform
} // namespace image
} // namespace astro
