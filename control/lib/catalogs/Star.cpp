/*
 * Star.cpp -- implementation of the star class
 *
 * (c) 2015 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroFormat.h>

namespace astro {
namespace catalog {

bool	Star::isDuplicate() const {
	return (_duplicate != '\0');
}

char	Star::duplicateCatalog() const {
	return _duplicate;
}

const std::string& Star::duplicatename() const {
	if (!isDuplicate()) {
		throw std::runtime_error("no duplicate star name available");
	}
	return _duplicatename;
}

void	Star::setDuplicate(char catalog, const std::string& name) {
	_duplicate = catalog;
	_duplicatename = name;
}

std::string	Star::toString() const {
        return stringprintf("%s %s %.2f %s",
                        ra().hms().c_str(), dec().dms().c_str(), mag(),
			longname().c_str());
}

} // namespace catalog
} // namespace astro
