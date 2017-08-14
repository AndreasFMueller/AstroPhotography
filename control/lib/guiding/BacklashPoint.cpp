/*
 * BacklashPoint.cpp -- methods associated with Backlash raw data
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Backlash.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {
namespace guiding {

/**
 * \brief Convert a backlash Point to a string representation
 */
std::string	BacklashPoint::toString() const {
	return stringprintf("%d: time=%.1f, x=%.1f, y=%.1f", id, time,
		xoffset, yoffset);
}

/**
 * \brief Write the backasch point as a comma separated line to a stream
 */
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

/**
 * \brief Write a whole vector of BacklashPoints to an output stream
 */
std::ostream&	operator<<(std::ostream& out,
			const std::vector<BacklashPoint>& points) {
	out << "id,time,xoffset,yoffset" << std::endl;
	std::for_each(points.begin(), points.end(),
		[&out](const BacklashPoint& p) mutable {
			out << p << std::endl;
		}
	);
	return out;
}

} // namespace guiding
} // namespace astro
