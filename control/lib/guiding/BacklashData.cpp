/*
 * BacklashData.cpp -- backlash data container
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <Backlash.h>

namespace astro {
namespace guiding {

std::ostream&	operator<<(std::ostream& out, const BacklashData& bd) {
	out << bd.result << std::endl;
	out << bd.points;
	return out;
}

} // namespace guiding
} // namespace astro
