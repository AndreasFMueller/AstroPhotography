//
// instrument.ice -- Interface to instruments over ZeroC
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil 2013
//
#include <camera.ice>
#include <Ice/Identity.ice>

module snowstar {
	/**
	 * \brief Instrument component types
	 *
	 * Not all device types can also be instrument components, e.g.
	 * it does not make sense to have cameras as components, we only need
	 * to talk about CCDs and their coolers or guider ports.
	 */
	enum InstrumentComponentType {
		InstrumentAdaptiveOptics = 0,
		InstrumentCamera = 1,
		InstrumentCCD = 2,
		InstrumentCooler = 3,
		InstrumentGuiderCCD = 4,
		InstrumentFinderCCD = 5,
		InstrumentGuidePort = 6,
		InstrumentFilterWheel = 7,
		InstrumentFocuser = 8,
		InstrumentMount = 9
	};

	/**
 	 * \brief Information about a component
	 */
	struct InstrumentComponent {
		// name of the instrument this component belongs to
		string	instrumentname;
		// the type of the component
		InstrumentComponentType	type;
		// if the instruments has multiple instruments, then we want
		// a way to identify the component
		int	index;
		// name of the server as published in mDNS
		string	servicename;
		// device url on the server (i.e. a local name)
		string	deviceurl;
	};

	sequence<InstrumentComponent>	InstrumentComponentList;

	struct InstrumentProperty {
		string instrumentname;
		string property;
		string value;
		string description;
	};
	sequence<string>	InstrumentPropertyNames;
	sequence<InstrumentProperty>	InstrumentPropertyList;

	/**
	 * \brief Instrument interface
	 */
	interface Instrument extends Statistics {
		string	name();

		/**
		 * \brief Get the number of components of a given type
		 *
		 * The return value may be zero if the instrument does not
		 * have any components of the given type.
		 */
		int	nComponentsOfType(InstrumentComponentType component)
				throws BadParameter;

		/**
		 * \brief Get component information for given type and index
		 *
		 * The component is identified by the type and the index
		 */
		InstrumentComponent	getComponent(
			InstrumentComponentType component, int index)
				throws BadParameter, NotFound;

		/**
		 * \brief Add a new component
		 *
		 * The index attribute of the component is ignored, and the
		 * the index of the component added is returned.
		 */
		int	add(InstrumentComponent component)
				throws BadParameter;

		/**
		 * \brief Update the instrument component
		 *
		 * The component to update is identified by the type and
		 * the index.
		 */
		void	update(InstrumentComponent component)
				throws BadParameter, NotFound;

		/**
		 * \brief Remove a component
		 */
		void	remove(InstrumentComponentType type, int index) throws NotFound;

		/**
		 * \brief Return a list of all components of the instrument
		 *
		 * This method gives a list of all components of this
		 * instrument, which may reduce the number of server
		 * round-trips that would be necessary with the other methods.
		 */
		InstrumentComponentList	list();

		// methods related to properties
		void	addProperty(InstrumentProperty property);
		InstrumentProperty	getProperty(string property)
						throws NotFound;
		void	removeProperty(string property) throws NotFound;
		void	updateProperty(InstrumentProperty property)
						throws NotFound;
		InstrumentPropertyNames	getPropertyNames();
		InstrumentPropertyList	getProperties();
	};

	sequence<string>		InstrumentList;

	/**
	 * \brief Instrument server interface
	 *
	 * This interface allows to list available instruments, creating and
	 * deleting instruments. It also gives access to instrument instances.
	 */
	interface Instruments extends Statistics {
		InstrumentList	list();
		Instrument	*get(string name) throws NotFound;
		void	remove(string name) throws NotFound;
		bool	has(string name);
	};

};
