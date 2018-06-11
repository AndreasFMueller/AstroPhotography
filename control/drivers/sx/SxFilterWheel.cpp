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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "opening filter wheel with name %s",
		name.toString().c_str());
	// extract the serial number from the name
	std::string	serial = name.unitname();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "serial number: %s", serial.c_str());

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
		std::string	serial_number("080");
		if (p->serial_number) {
			serial_number = wchar2string(p->serial_number);
		}
		if (serial == serial_number) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "opening HID device");
			_hid = hid_open(p->vendor_id, p->product_id,
				p->serial_number);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "hid = %p", _hid);
			break;
		}
		p = p->next;
	}
	hid_free_enumeration(hinfo);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "enumeration of HID devices complete");

	// initialize the state variables
	pending_cmd = no_command;
	state = unknown;
	currentposition = 0;

	// number of filters
	nfilters = 0;

	// make the hid device nonblocking
	hid_set_nonblocking(_hid, 1);

	// send the get total command
	send_command(get_total);
}

//	// initialize the filter names
//	for (size_t i = 0; i < nfilters; i++) {
//		filternames.push_back(stringprintf("filter-%d", i));
//	}
//}

/**
 * \brief Destroy the FilterWheel object
 */
SxFilterWheel::~SxFilterWheel() {
	// close connection to the filter wheel
	hid_close(_hid);
	_hid = NULL;
}

/**
 * \brief Auxiliary function to send a new command
 *
 * \param cmd	command to send
 * \param arg	argument to command
 */
void	SxFilterWheel::send_command(filterwheel_cmd_t cmd, int arg) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got command %d/%d for filterwheel",
		cmd, arg);
	if (cmd == no_command) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no command");
		return;
	}

	// make sure the previous command did complete
	if (pending_cmd != no_command) {
		std::string	msg = stringprintf("pending command: %d",
			pending_cmd);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}

	// send a new command
	uint8_t	buffer[3];
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	switch (cmd) {
	case select_filter:
		buffer[1] = arg;
		break;
	case current_filter:
		break;
	case get_total:
		buffer[2] = 1;
		break;
	case no_command:
		// should not happen
		throw std::logic_error("cannot happen");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sending %02x,%02x report",
		buffer[1], buffer[2]);
	int	rc = hid_write(_hid, buffer, 3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d bytes written", rc);
	if (rc != 2) {
		std::string	msg = stringprintf("cannot write command: %s ",
			hid_error(_hid));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	pending_cmd = cmd;
	switch (pending_cmd) {
	case select_filter:
	case get_total:
		state = moving;
		break;
	case current_filter:
		break;
	case no_command:
		// should not happen
		throw std::logic_error("cannot happen");
	}
}

/**
 * \brief Read the response
 *
 * \return -1 if there is no response, 0 if filterhweel is moving and > 0
 */
int	SxFilterWheel::read_response() {
	int	result = -1;
	if (pending_cmd == no_command) {
		return result;
	}
	uint8_t	buffer[3];
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	int	rc = hid_read(_hid, buffer, 3);
	if (rc < 0) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot read: %s",
			hid_error(_hid));
		return -1;
	}
	if (2 != rc) {
		std::string	msg = stringprintf("wrong number of "
			"bytes: %d != 2", rc);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return buffer[1];
}

/**
 * \brief Try to complete an open command
 */
int	SxFilterWheel::try_complete() {
	// if there is no pending command, return
	if (pending_cmd == no_command) {
		return 0;
	}
	// if there is a pending command, try to get a response
	int	a = read_response();
	if (a < 0) {
		// no response
		return 0;
	}
	if (a == 0) {
		// still pending
		return 0;
	}
	switch (pending_cmd) {
	case select_filter:
	case current_filter:
		currentposition = a;
		break;
	case get_total:
		nfilters = a;
		currentposition = 1;
		// initialize the filter names
		for (size_t i = 0; i < nfilters; i++) {
			filternames.push_back(stringprintf("filter-%d", i));
		}
		break;
	case no_command:
		// should not happen
		throw std::logic_error("cannot happen");
	}
	state = idle;
	return 1;
}

unsigned int	SxFilterWheel::nFilters() {
	if (nfilters <= 0) {
		try_complete();
	}
	if (nfilters > 0) {
		return nfilters;
	}
	throw BadState("filterwheel not idle");
}

/**
 * \brief Get the 
 */
unsigned int	SxFilterWheel::currentPosition() {
	if (state == idle) {
		return currentposition;
	}
	try_complete();
	if (state != idle) {
		throw BadState("filter wheel busy");
	}
	return currentposition;
} 

/**
 * \brief select a filter by its number
 *
 * \param filterindex	number of the filter to select
 */
void	SxFilterWheel::select(size_t filterindex) {
	// make sure we can send a command
	if (pending_cmd != no_command) {
		std::string	msg = stringprintf("filterwheel busy");
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}

	// send a new command
	send_command(select_filter, filterindex + 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "send select filter %d sent",
		filterindex + 1);
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
	// check whether we have a pending command
	if (pending_cmd == no_command) {
		return FilterWheel::idle;
	}
	try_complete();
	switch (state) {
	case unknown:
		return FilterWheel::unknown;
	case moving:
		return FilterWheel::moving;
	case idle:
		return FilterWheel::idle;
	}
}

} // namespace sx
} // namespace camera
} // namespace astro
