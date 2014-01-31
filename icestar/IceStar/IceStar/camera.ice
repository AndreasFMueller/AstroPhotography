//
// camera.ice -- Interface definition for camera access
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <image.ice>

/**
 * \brief snowstar module captures all interfaces
 */
module snowstar {
	// exposure related data structures
	struct BinningMode {
		long	x;
		long	y;
	};

	sequence<BinningMode>	BinningSet;

	enum ShutterState { ShCLOSED, ShOPEN };

	/**
	 * \brief Exposure request structure
	 */
	struct Exposure {
		/**
		 * \brief CCD subrectangle to retrieve.
		 *
		 * If the size field of the frame is set to (0,0), it is assumed
		 * that the complete CCD is returned.
		 */
		ImageRectangle	frame;
		/**
		 * \brief Exposure time in seconds.
		 */
		float	exposuretime;
		/**
		 * \brief Gain.
		 *
		 * Many cameras do not support the gain setting, so by default,
		 * the gain should be set to 1.
		 */
		float	gain;
		/**
		 * \brief Limit for pixel values.
		 *
		 * Some cameras may produce confusingly large pixel values in
		 * some cases. To simplify image interpretation, the exposure
		 * methods can use the limit member to limit the pixel values.
		 */
		float	limit;
		/**
		 * \brief Shutter state during exposure.
		 *
		 * For cameras that have a shutter, this setting indicates
		 * whether the shutter should be open or closed during exposure.
		 * For dark images, the shutter needs to be closed.
		 */
		ShutterState	shutter;
		/**
		 * \brief Binning mode to use during readout
		 *
		 * Default binning mode should always be 1x1. If the binning
		 * mode is 0x0, 1x1 is assumed.
		 */
		BinningMode	mode;
	};

	// Device objects
	/**
	 * \brief Info about a CCD
	 *
	 * This contains information that can typically be retrieved without
	 * accessing the CCD.
	 */
	struct CcdInfo {
		string	name;
		long	id;
		ImageSize	size;
		BinningSet	binningmodes;
		bool	shutter;
		float	pixelwidth;
		float	pixelheight;
	};

	interface Cooler;

	enum ExposureState { IDLE, EXPOSING, EXPOSED, CANCELLING };

	/**
	 * \brief Interface to a CCD chip of a camera.
	 *
	 * A camera can have multiple CCD chips (up to three, e.g. in the
	 * case of the SBIG STX-16803). Each CCD can have a cooler.
	 * It is also assumed, that each CCD can have a shutter, although
	 * for some cameras, a shutter may actually affect more than one
	 * CCD (e.g. with self guiding cameras, if the guiging CCD is
	 * behind the shutter.
	 */
	interface Ccd {
		/**	
		 * \brief get the name of the ccd
		 */
		string	getName();
		/**
 		 * \brief get the CcdInfo from the ccd
		 */
		CcdInfo	getInfo();
		/**
		 * \brief Start an exposure
		 */
		void	startExposure(Exposure exp)
				throws BadState, BadParameter;
		/**
		 * \brief Find the state of the exposure
		 */
		ExposureState	exposureStatus();

		/**
		 * \brief Find out how long ago the last exposure was started
		 */
		long	lastExposureStart();

		/**
		 * \brief Cancel an exposure
		 */
		void	cancelExposure() throws BadState, NotImplemented;
		/**
		 * \brief Get information about the exposure
		 *
		 * This only makes sense if an exposure is in progres or
		 * has completed. In all other cases, a BadState exception
		 * is raised.
		 */
		Exposure	getExposure() throws BadState;
		/**
		 * \brief Retrieve the image
		 *
		 * For this method to work the Ccd must be in state exposed.
		 * Retreiving the image will update the state to idle.
		 */
		Image	getImage() throws BadState;

		/**
		 * \brief Find out whether this CCD has a gain setting.
		 */
		bool	hasGain();
		/**
		 * \brief Find out whether this CCD has a shutter
		 */
		bool	hasShutter();
		/**
		 * \brief Get the current shutter state
		 */
		ShutterState	getShutterState() throws NotImplemented;
		/**
		 * \brief Set the current shutter state
		 */
		void	setShutterState(ShutterState state)
				throws NotImplemented;
		/**
		 * \brief Find out whether the CCD has a cooler
		 */
		bool	hasCooler();
		/**
		 * \brief Get the Cooler
		 */
		Cooler	getCooler() throws NotImplemented;
	};

	/**
	 * \brief Thermoelectric coolers
	 *
	 * Some CCDs have thermoelectric coolers. This interface allows
	 * to control their temperature.
	 */
	interface Cooler {
		/**
		 * \brief get the name of the cooler
		 */
		string	getName();
		/**
		 * \brief Get temperature at which the cooler is set
		 */
		float	getSetTemperature();
		/**
		 * \brief Get the actual temperature of the CCD
		 */
		float	getActualTemperature();
		/**
		 * \brief Set the temperature.
		 *
		 * This does not automatically mean that the temperature
		 * has been reached. For this one should query the actual
		 * temperature using getActualTemperature() repeatedly 
		 * until the temperature has been reached to a sufficiently
		 * accurate degree.
	 	 */
		void	setTemperature(float temperature);
		/**
		 * \brief Find out whether cooler is on.
		 */
		bool	isOn();
		/**
		 * \brief Turn cooler on.
		 *
		 * Note that setting the temperature on does not automatically
		 * turn the cooler on, at least on some devices. To get cooling,
		 * set the temperature AND set the it on.
		 */
		void	setOn(bool onoff);
	};

	/**
	 * \brief Interface for Guider Ports
	 *
	 * The guider port has four outputs, two for right ascension
	 * (RA+ and RA- for west and east) and two for declination (DEC+
	 * DEC- for north and south). Activating the outputs changes the
	 * telescope movement in the corresponding direction.
	 * This interface allows to control activation of these ports.
	 */
	const byte	DECMINUS = 1;
	const byte	DECPLUS = 2;
	const byte	RAMINUS = 4;
	const byte	RAPLUS = 8;
	interface GuiderPort {
		/**
		 * \brief get the name of the guiderport
		 */
		string	getName();

		/**
		 * \brief Retrieve active ports
		 *
		 * Which ports are currently active
		 */
		byte	active();
		/**
		 * \brief Activate Guider Port outputs.
		 *
		 * This method activates the guider port outputs for the
		 * time specified in the arguments. A positive argument
		 * activates the + output, a negative argument the - output.
		 * \param ra	Time in seconds to activate the RA outputs
		 * \param dec	Time in seconds to activate the DEC outputs
	 	 */
		void	activate(float ra, float dec);
	};


	/**
	 * \brief State of the filterwheel
	 *
	 * The idle state means that the filterwheel is now in a known
	 * position that can be queried via the currentPosition method.
	 * When a filterwheel initializes, it will be either in the unknown
	 * or the moving state. New position requests can be issued when
	 * the filter wheel is in the idle or unknown state. One then should
	 * wait until it is in the idle state, 
	 */
	enum FilterwheelState { FwIDLE, FwMOVING, FwUNKNOWN };

	/**
 	 * \brief FilterWheel interface
	 *
	 * A Filterwheel is a device that can position a certain number
	 * of filters into the light path of the camera.
	 */
	interface FilterWheel {
		/**
 		 * \brief get the name of the filter wheel
		 */
		string	getName();
		/**
		 * \brief Number of available filter positions.
		 *
		 * This method also counts empty filter positions in the wheel.
		 */
		long	nFilters();
		/**
		 * \brief Query the current filter position.
		 */
		long	currentPosition();
		/**
		 * \brief Move the filter wheel to a given position
		 */
		void	select(long position) throws NotFound;
		/**
		 * \brief Get the name of the filter
		 */
		string	filterName(long position) throws NotFound;
		/**
		 * \brief Query the filter wheel state
		 */
		FilterwheelState	getState();
	};

	/**
	 * \brief Focuser abstraction
	 */
	interface Focuser {
		string	getName();
		long	min();
		long	max();
		long	current();
		void	set(long value);
	};

	/**
	 * \brief Camera abstraction
	 *
	 * A camera consists of a number of CCDs that can be controlled
	 * individually. It can also have Filterwheels attached, and many
	 * cameras have a guider port.
	 */
	interface Camera {
		/**
		 * \brief Get the camera name.
		 */
		string	getName();
		// CcdInfo info;
		/**
		 * \brief Find out how many CCDs the camera has
	 	 */
		long	nCcds();
		/**
		 * \brief Get Information about the CCD.
		 *
		 * This method does not return all available information, but
		 * typically only information available without accessing
		 * the CCD. It should be sufficient to plan an exposure.
		 */
		CcdInfo	getCcdinfo(long ccdid) throws NotFound;
		/**
		 * \brief Retrieve a CCD.
		 */
		Ccd	getCcd(long ccdid) throws NotFound;
		// FilterWheel
		/**
		 * \brief Find out whether the camera has FilterWheel
		 */
		bool	hasFilterWheel();
		/**
		 * \brief Get the FilterWheel
		 */
		FilterWheel	getFilterWheel() throws NotImplemented;
		// Guider Port
		/**
		 * \brief Find out whether the camera has a guider port.
	 	 */
		bool	hasGuiderPort();
		/**
		 * \brief Get the Guider Port
		 */
		GuiderPort	getGuiderPort() throws NotImplemented;
	};
};
