//
// exceptions.ice -- Interface definition for device access
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//

/**
 * \brief snowstar module captures all interfaces
 */
module snowstar {
	// Exceptions
	/**
	 * \brief Exception for requests during wrong state
	 *
	 * The CCD interface implements a state machine, certain requests
	 * are only possible in some states. E.g. it is not possible to
	 * start a new exposure if one is already in progress. Trying to
	 * do so raises the BadState exception.
	 */
	exception BadState {
		string cause;
	};

	/**
	 * \brief Exception for bad parameters
	 */
	exception BadParameter {
		string cause;
	};

	/**
	 * \brief An object was not found
	 */
	exception NotFound {
		string cause;
	};

	/**
	 * \brief The device in question does not implement this function
	 */
	exception NotImplemented {
		string cause;
	};

	/**
	 * \brief Exception thrown when the servant encouters an I/O problem
	 */
	exception IOException {
		string cause;
	};

	/**
 	 * \brief DeviceException
	 */
	exception DeviceException {
		string cause;
	};

	/**
	 * \brief Exception thrown when an object already exists
 	 */
	exception Exists {
		string cause;
	};
};
