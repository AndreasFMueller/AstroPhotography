/*
 * SxFilterWheel.cpp -- driver for the filter wheel
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SxFilterWheel.h>
#include <AstroExceptions.h>
#include <SxUtils.h>
#include <includes.h>
#include "sx.h"

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief tranmpoline function to jump into the thread run function
 */
static void	filterwheel_main(SxFilterWheel *filterwheel) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "starting filterwheel_main(%s)",
		filterwheel->name().toString().c_str());
	try {
		filterwheel->run();
	} catch (std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error in filterwheel_main: %s",
			x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filterwheel_main terminated");
}

/**
 * \brief Thread main method
 */
void	SxFilterWheel::run() {
	_barrier.await();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "SxFilterWheel:run() start");
	uint8_t	command[3];
	uint8_t	response[3];
	int	rc = 0;
	while (!_terminate) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "try again");
		if (pending_cmd == no_command) {
			goto waitnext;
		}

		// send the command
		memset(command, 0, sizeof(command));
		switch (pending_cmd) {
		case select_filter:
			command[1] = currentposition;
			break;
		case current_filter:
			break;
		case get_total:
			command[2] = 1;
			pending_cmd = current_filter;
			break;
		case no_command:
			break;
		}

		debug(LOG_DEBUG, DEBUG_LOG, 0, "sending %02x,%02x report",
			command[1], command[2]);
		rc = hid_write(_hid, command, 3);
		if (rc != 3) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "failed to send: %d",
				rc);
			usleep(100000);
			continue;
		}

		// wait for response
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got a response: %d", rc);
		memset(response, 0, sizeof(response));
		rc = hid_read(_hid, response, 2);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "response (%d): %02x,%02x,%02x",
			rc, response[0], response[1], response[2]);
		if (rc < 0) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"bad response, skipping");
			usleep(100000);
			continue;
		}
		
		// what to do depening in the pending command
		if (response[0] == 0) {
			usleep(100000);
			continue;
		}
		switch (pending_cmd) {
		case select_filter:
			pending_cmd = current_filter;
			continue;
			break;
		case get_total:
		case current_filter:
			currentposition = response[0];
			nfilters = response[1];
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"current = %d, total = %d",
				currentposition, nfilters);
			break;
		case no_command:
			debug(LOG_DEBUG, DEBUG_LOG, 0, "should not happen");
			break;
		}

		// add the filter names if there aren't any names
		if ((filternames.size() == 0) && (nfilters > 0)) {
			std::string	devname = name().toString();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "get properties for '%s'",
				devname.c_str());
			Properties	properties(devname);

			for (size_t i = 1; i <= nfilters; i++) {
				std::string n = stringprintf("filter-%lu", i);
				if (properties.hasProperty(n)) {
					n = properties.getProperty(n);
				}
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"set filter %lu name %s", i, n.c_str());
				filternames.push_back(n);
			}

		}

	waitnext:
		debug(LOG_DEBUG, DEBUG_LOG, 0, "command complete");
		// reset the command
		{
			std::unique_lock<std::recursive_mutex>	lock(_mutex);
			
			pending_cmd = no_command;
			state = idle;

			// wait for the new command
			_condition.wait(lock);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "SxFilterWheel:run() end");
}

/**
 * \brief Construct a filterwheel object
 *
 * \param name	name of the filterwheel
 */
SxFilterWheel::SxFilterWheel(const DeviceName& name)
	: FilterWheel(name), _barrier(2) {
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found HID device %p", hinfo);
	struct hid_device_info	*p = hinfo;
	while (p) {
		std::string	serial_number("080");
		if (p->serial_number) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "converting serial number");
			std::string	s = wchar2string(p->serial_number);
			if (s.size() > 0) {
				serial_number = s;
			}
		} else {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "no serial");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "going to open the HID device");
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
	pending_cmd = get_total;
	state = unknown;
	currentposition = 0;

	// number of filters
	nfilters = 0;

	// start the thread
	_terminate = false;
	_thread = new std::thread(filterwheel_main, this);

	// release the thread
	_barrier.await(); // release the thread
}

/**
 * \brief Destroy the FilterWheel object
 */
SxFilterWheel::~SxFilterWheel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy FilterWheel");
	// wait for the thread to terminate
	_terminate = true;
	_condition.notify_all();
	_thread->join();
	delete _thread;

	// close connection to the filter wheel
	hid_close(_hid);
	_hid = NULL;
}

unsigned int	SxFilterWheel::nFilters() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (nfilters > 0) {
		return nfilters;
	}
	throw BadState("filterwheel not idle");
}

/**
 * \brief Get the current position of the filterwheel
 */
unsigned int	SxFilterWheel::currentPosition() {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	if (state == idle) {
		return currentposition - 1;
	}
	throw BadState("filter wheel busy");
} 

/**
 * \brief select a filter by its number
 *
 * \param filterindex	number of the filter to select
 */
void	SxFilterWheel::select(size_t filterindex) {
	std::unique_lock<std::recursive_mutex>	lock(_mutex);
	// make sure we can send a command
	if (pending_cmd != no_command) {
		std::string	msg = stringprintf("filterwheel busy: %d",
			pending_cmd);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw BadState(msg);
	}

	// send a new command
	pending_cmd = select_filter;
	state = moving;
	currentposition = filterindex + 1;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "send select filter %d",
		filterindex + 1);
	_condition.notify_all();
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
	switch (state) {
	case unknown:
		return FilterWheel::unknown;
	case moving:
		return FilterWheel::moving;
	case idle:
		return FilterWheel::idle;
	}
	throw std::logic_error("should not happen");
}

/**
 * \brief User friendly name of the filter wheel
 */
std::string	SxFilterWheel::userFriendlyName() const {
	return std::string("Starlight Express USB FilterWheel");
}

} // namespace sx
} // namespace camera
} // namespace astro
