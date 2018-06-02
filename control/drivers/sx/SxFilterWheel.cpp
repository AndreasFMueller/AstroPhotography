/*
 * SxFilterWheel.cpp -- driver for the filter wheel
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SxFilterWheel.h>
#include <AstroExceptions.h>

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Construct a filterwheel object
 */
SxFilterWheel::SxFilterWheel(const DeviceName& name) : FilterWheel(name) {
	// initialize connection to the filter wheel

	// find out how many filters there are
	nfilters = 7;

	// initialize the filter names
	for (size_t i = 0; i < nfilters; i++) {
		filternames.push_back(stringprintf("filter-%d", i));
	}
}

/**
 * \brief Destroy the FilterWheel object
 */
SxFilterWheel::~SxFilterWheel() {
	// XXX close connection to the filter wheel
}

unsigned int	SxFilterWheel::nFilters() {
	return nfilters;
}

/**
 * \brief Get the 
 */
unsigned int	SxFilterWheel::currentPosition() {
	// XXX query the filter wheel
	return 0;
}

/**
 * \brief select a filter by its number
 *
 * \param filterindex	number of the filter to select
 */
void	SxFilterWheel::select(size_t filterindex) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting filter %d", filterindex);
	// XXX tell the filter wheel what filter to select
}

/**
 * \brief select a filte rby name
 *
 * \param filtername	name of the filter
 */
void	SxFilterWheel::select(const std::string& filtername) {
	for (size_t i = 0; i < nfilters; i++) {
		if (filtername == filternames[i]) {
			select(i);
			return;
		}
	}
	std::string	msg = stringprintf("filter '%s' not found",
				filtername.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw NotFound(msg);
}

/**
 * \brief get the name of the filter 
 *
 * \param filterindex	number of the filter
 */
std::string	SxFilterWheel::filterName(size_t filterindex) {
	if (filterindex >= nfilters) {
		std::string	msg = stringprintf("filter %d does not exist",
			filterindex);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadParameter(msg);
	}
	return  filternames[filterindex];
}

/**
 * \brief Get the filter wheel state
 */
FilterWheel::State	SxFilterWheel::getState() {
	// query the filter wheel
	return FilterWheel::idle;
}

} // namespace sx
} // namespace camera
} // namespace astro
