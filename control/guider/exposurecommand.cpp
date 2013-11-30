/*
 * exposurecommand.cpp -- tools for parsing exposure commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <exposurecommand.h>
#include <sstream>

namespace astro {
namespace cli {

ExposureParser::ExposureParser() {
}

ExposureParser::ExposureParser(const astro::camera::CcdInfo& info) {
	_exposure.frame.setSize(info.size());
}

ExposureParser::ExposureParser(const astro::camera::Exposure& exposure)
	: _exposure(exposure) {
}

/**
 * \brief Parse a single argument
 */
void	ExposureParser::operator()(const std::string& valuepair) {
	// check whether this is actually an attribute=value pair
	size_t	offset = valuepair.find('=');
	if (std::string::npos == offset) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not an attribute=value pair: %s",
			valuepair.c_str());
		return;
	}

	// separate attribute and value
	std::string	attribute = valuepair.substr(0, offset);
	std::string	value = valuepair.substr(offset + 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "attr-value-pair: %s=%s",
		attribute.c_str(), value.c_str());

	// now analyze the pair
	std::istringstream	in(value);
	if ("origin" == attribute) {
		astro::image::ImagePoint	origin;
		in >> origin;
		_exposure.frame.setOrigin(origin);
		return;
	}
	if ("size" == attribute) {
		astro::image::ImageSize	size;
		in >> size;
		_exposure.frame.setSize(size);
		return;
	}
	if ("binning" == attribute) {
		astro::camera::Binning	binning;
		in >> binning;
		_exposure.mode = binning;
		return;
	}
	if ("exposuretime" == attribute) {
		in >> _exposure.exposuretime;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "set exposure time: %f",
			_exposure.exposuretime);
		return;
	}
	if ("gain" == attribute) {
		in >> _exposure.gain;
	}
	if ("limit" == attribute) {
		in >> _exposure.limit;
	}
	if ("shutter" == attribute) {
		if ("closed" == value) {
			_exposure.shutter = astro::camera::SHUTTER_CLOSED;
			return;
		}
		if ("open" == value) {
			_exposure.shutter = astro::camera::SHUTTER_OPEN;
			return;
		}
		debug(LOG_ERR, DEBUG_LOG, 0, "bad shutter value: %s",
			value.c_str());
		throw std::runtime_error("bad shutter value");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unknown attribute: %s",
		attribute.c_str());
	throw std::runtime_error("unknown attribute");
}

void	ExposureParser::parse(const std::vector<std::string>& arguments,
		unsigned int offset) {
	std::vector<std::string>::const_iterator	i = arguments.begin();

	// skip the arguments below the offset
	while ((offset--) && (i != arguments.end())) {
		i++;
	}

	// apply the operator() to every argument
	_exposure = std::for_each(i, arguments.end(), *this).exposure();
}

} // namespace cli
} // namespace astro
