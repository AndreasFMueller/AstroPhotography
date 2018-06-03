/*
 * SxFilterWheel.cpp -- driver for the filter wheel
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SxFilterWheel.h>
#include <AstroExceptions.h>
#include <SxUtils.h>
#include "sx.h"

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Construct a filterwheel object
 *
 * \param name	name of the filterwheel
 */
SxFilterWheel::SxFilterWheel(const DeviceName& name) : FilterWheel(name) {
	// extract the serial number from the name
	std::string	serial = name.unitname();

	// initialize connection to the filter wheel
	struct hid_device_info	*hinfo = hid_enumerate(SX_VENDOR_ID,
		SX_FILTERWHEEL_PRODUCT_ID);
	if (NULL == hinfo) {
		std::string	msg("SX Filterwheel not found");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	struct hid_device_info	*p = hinfo;
	while (p) {
		std::string	serial_number = wchar2string(p->serial_number);
		if (serial == serial_number) {
			_hid = hid_open(p->vendor_id, p->product_id,
				p->serial_number);
			break;
		}
		p = p->next;
	}
	hid_free_enumeration(hinfo);

	// find out how many filters there are
	uint8_t	buffer[2];
	buffer[0] = 0;
	buffer[1] = 1;
	if (2 != hid_write(_hid, buffer, 2)) {
		std::string	msg = stringprintf("cannot query number of "
			"filters: %s", hid_error(_hid));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	nfilters = 0;
	int	rc;
	do {
		rc = hid_read(_hid, buffer, 2);
		if (rc < 0) {
			std::string	msg = stringprintf("cannot read: %s",
				hid_error(_hid));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		if (2 != rc) {
			std::string	msg = stringprintf("wrong number of "
				"bytes: %d != 2", rc);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		if (buffer[0] > 0) {
			nfilters = buffer[0];
		}
	} while (nfilters <= 0);

	// initialize the filter names
	for (size_t i = 0; i < nfilters; i++) {
		filternames.push_back(stringprintf("filter-%d", i));
	}
}

/**
 * \brief Destroy the FilterWheel object
 */
SxFilterWheel::~SxFilterWheel() {
	// close connection to the filter wheel
	hid_close(_hid);
	_hid = NULL;
}

unsigned int	SxFilterWheel::nFilters() {
	return nfilters;
}

/**
 * \brief Get the 
 */
unsigned int	SxFilterWheel::currentPosition() {
	// return current filter
	uint8_t	buffer[2];
	buffer[0] = 0;
	buffer[1] = 0;
	int	rc = hid_write(_hid, buffer, 2);
	if (rc != 2) {
		std::string	msg = stringprintf("cannot write command: %s",
			hid_error(_hid));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	unsigned int	currentposition = 0;
	do {
		rc = hid_read(_hid, buffer, 2);
		if (rc < 0) {
			std::string	msg = stringprintf("cannot read: %s",
				hid_error(_hid));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		if (2 != rc) {
			std::string	msg = stringprintf("wrong number of "
				"bytes: %d != 2", rc);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		if (buffer[0] != 0) {
			currentposition = buffer[0];
		}
	} while (currentposition <= 0);
	return currentposition;
} 

/**
 * \brief select a filter by its number
 *
 * \param filterindex	number of the filter to select
 */
void	SxFilterWheel::select(size_t filterindex) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting filter %d", filterindex);
	// tell the filter wheel what filter to select
	uint8_t	buffer[2];
	buffer[0] = filterindex + 1;
	buffer[1] = 0;
	int	rc = hid_write(_hid, buffer, 2);
	if (rc != 2) {
		std::string	msg = stringprintf("cannot write command: %s",
			hid_error(_hid));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	unsigned int	currentposition = 0;
	do {
		rc = hid_read(_hid, buffer, 2);
		if (rc < 0) {
			std::string	msg = stringprintf("cannot read: %s",
				hid_error(_hid));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		if (2 != rc) {
			std::string	msg = stringprintf("wrong number of "
				"bytes: %d != 2", rc);
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		if (buffer[0] != 0) {
			currentposition = buffer[0];
		}
	} while (currentposition <= 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "position %d reached", currentposition);
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
