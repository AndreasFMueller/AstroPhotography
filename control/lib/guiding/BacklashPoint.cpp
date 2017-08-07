/*
 * BacklashPoint.h -- methods associated with Backlash raw data
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Backlash.h>

namespace astro {
namespace guiding {

std::ostream&	operator<<(std::ostream& out, const BacklashPoint& point) {
	out << point.id;
	out << ",";
	out << point.time;
	out << ",";
	out << point.xoffset;
	out << ",";
	out << point.yoffset;
	return out;
}

std::ostream&	operator<<(std::ostream& out,
			const std::vector<BacklashPoint>& points) {
	std::for_each(points.begin(), points.end(),
		[&out](const BacklashPoint& p) mutable {
			out << p << std::endl;
		}
	);
	return out;
}

} // namespace guiding
} // namespace astro
