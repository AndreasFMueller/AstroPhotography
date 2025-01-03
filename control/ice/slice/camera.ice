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
	sequence<BinningMode>	BinningSet;

	enum ShutterState { ShCLOSED, ShOPEN };
	enum ExposurePurpose {
		ExLIGHT,
		ExDARK,
		ExFLAT,
		ExBIAS,
		ExTEST,
		ExGUIDE,
		ExFOCUS,
		ExFLOOD,
		ExPREVIEW
	};
	enum ExposureQuality {
		ExQualityHIGH,
		ExQualityFAST
	};

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
		 * \brief Exposure purpose
		 *
		 * The camera may behave differently if it knows what the
		 * purpose of the exposure is.
		 */
		ExposurePurpose	purpose;
		/**
		 * \brief Binning mode to use during readout
		 *
		 * Default binning mode should always be 1x1. If the binning
		 * mode is 0x0, 1x1 is assumed.
		 */
		BinningMode	mode;
		/**
		 * \brief The exposure quality
		 *
		 * The quality can be either high or the image can be retrieved
		 * quickly.
		 */
		ExposureQuality	quality;
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
		int	id;
		ImageSize	size;
		BinningSet	binningmodes;
		bool	shutter;
		float	pixelwidth;
		float	pixelheight;
		float	minexposuretime;
		float	maxexposuretime;
	};

	interface Cooler;

	enum ExposureState {
		IDLE,
		EXPOSING,
		EXPOSED,
		CANCELLING,
		STREAMING,
		BROKEN
	};

	struct Interval {
		float	min;
		float	max;
	};

	/**
	 * \brief Unit of data returned in streaming mode
	 *
	 * Because the parameters can change during streaming, we have to
	 * return more than just the image. This structure contains the
	 * exposure data as a member.
	 */
	struct ImageQueueEntry {
		Exposure	exposure0;
		ImageFile	imagedata;
	};

	/**
	 * \brief Callback interface used to stream images to the client
	 *
 	 * A single callback can be registered with the server. The server
	 * in turn uses 
	 */
	interface ImageSink extends Callback {
		void	image(ImageQueueEntry entry);
	};

	/**
	 * \brief Callback interface for CCD state changes
	 */
	interface CcdCallback extends Callback {
		void	state(ExposureState s);
	}

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
	interface Ccd extends Device {
		/**
 		 * \brief get the CcdInfo from the ccd
		 */
		CcdInfo	getInfo() throws DeviceException;
		/**
		 * \brief Start an exposure
		 */
		void	startExposure(Exposure exp)
				throws BadState, BadParameter, DeviceException;
		/**
		 * \brief Find the state of the exposure
		 */
		ExposureState	exposureStatus();

		/**
		 * \brief Find out how long ago the last exposure was started
		 */
		int	lastExposureStart();

		/**
		 * \brief Cancel an exposure
		 */
		void	cancelExposure() throws BadState, NotImplemented,
						DeviceException;
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
		Image*	getImage() throws BadState, DeviceException;

		/**
		 * \brief Find out whether this CCD has a gain setting.
		 */
		bool	hasGain();
		/**
		 * \brief Retrieve the current Gain setting
		 */
		float	getGain();
		/**
		 * \brief get the interval of valid gain values
		 */
		Interval	gainInterval();
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
		Cooler*	getCooler() throws NotImplemented;

		/**
 		 * \brief Methods to register the state change callback
		 */
		void	registerCallback(Ice::Identity callback);
		void	unregisterCallback(Ice::Identity callback);

		/**
		 * \brief methods related to streaming
		 */
		void	registerSink(Ice::Identity i) throws NotImplemented;
		void	startStream(Exposure e) throws NotImplemented;
		void	updateStream(Exposure e) throws NotImplemented;
		void	stopStream() throws NotImplemented;
		void	unregisterSink() throws NotImplemented;

		/**
		 * \brief Find out whether this device is controllable
		 */
		bool	isControllable();
	};

	struct CoolerInfo {
		float	actualTemperature;
		float	setTemperature;
		bool	on;
	};

	interface CoolerCallback {
		void	updateCoolerInfo(CoolerInfo info);
		void	updateSetTemperature(float settemperature);
		void	updateDewHeater(float dewheater);
	};

	/**
	 * \brief Thermoelectric coolers
	 *
	 * Some CCDs have thermoelectric coolers. This interface allows
	 * to control their temperature.
	 */
	interface Cooler extends Device {
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
		/**
		 * \brief Check whether the cooler has a dew heater
		 */
		bool	hasDewHeater();
		/**
		 * \brief Get the current dew heater value
		 */
		float	getDewHeater();
		/**
		 * \brief set the dew heater value
		 */
		void	setDewHeater(float dewheatervalue);
		/**
		 * \brief Retrieve the range of valid dew heater values
		 */
		Interval	dewHeaterRange();

		void	registerCallback(Ice::Identity callback);
		void	unregisterCallback(Ice::Identity callback);
	};

	struct GuidePortActivation {
		float raplus;
		float raminus;
		float decplus;
		float decminus;
	};

	interface GuidePortCallback {
		void	activate(GuidePortActivation activation);
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
	interface GuidePort extends Device {
		/**
		 * \brief Retrieve active ports
		 *
		 * Which ports are currently active
		 */
		byte	active() throws DeviceException;
		/**
		 * \brief Activate Guider Port outputs.
		 *
		 * This method activates the guider port outputs for the
		 * time specified in the arguments. A positive argument
		 * activates the + output, a negative argument the - output.
		 * \param ra	Time in seconds to activate the RA outputs
		 * \param dec	Time in seconds to activate the DEC outputs
	 	 */
		void	activate(float ra, float dec) throws DeviceException;
		void	registerCallback(Ice::Identity callback);
		void	unregisterCallback(Ice::Identity callback);
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
	enum FilterwheelState {
		FwIDLE,
		FwMOVING,
		FwUNKNOWN
	};

	/**
	 * \brief Callback for filterwheel monitoring
	 */
	interface FilterWheelCallback extends Callback {
		void	state(FilterwheelState s);
		void	position(int filter);
	};

	/**
 	 * \brief FilterWheel interface
	 *
	 * A Filterwheel is a device that can position a certain number
	 * of filters into the light path of the camera.
	 */
	interface FilterWheel extends Device {
		/**
		 * \brief Number of available filter positions.
		 *
		 * This method also counts empty filter positions in the wheel.
		 */
		int	nFilters();
		/**
		 * \brief Query the current filter position.
		 */
		int	currentPosition();
		/**
		 * \brief Move the filter wheel to a given position
		 */
		void	select(int position) throws NotFound, DeviceException;
		/**
		 * \brief Move the filter wheel to a given filter name
		 */
		void	selectName(string filtername) throws NotFound, DeviceException;
		/**
		 * \brief Get the name of the filter
		 */
		string	filterName(int position) throws NotFound;
		/**
		 * \brief Query the filter wheel state
		 */
		FilterwheelState	getState();

		void	registerCallback(Ice::Identity callback);
		void	unregisterCallback(Ice::Identity callback);
	};

	/**
 	 * \brief Callback interface for focusers
	 */
	interface FocuserCallback extends Callback {
		void	movement(long fromposition, long toposition);
		void	info(long fromposition, bool ontarget);
	};

	/**
	 * \brief Focuser abstraction
	 *
	 * XXX this interface should also use callbacks
	 */
	interface Focuser extends Device {
		int	min();
		int	max();
		int	current();
		int	backlash();
		void	set(int value) throws DeviceException;
		void	registerCallback(Ice::Identity callback);
		void	unregisterCallback(Ice::Identity callback);
	};

	/**
	 * \brief Camera abstraction
	 *
	 * A camera consists of a number of CCDs that can be controlled
	 * individually. It can also have Filterwheels attached, and many
	 * cameras have a guider port.
	 */
	interface Camera extends Device {
		// CcdInfo info;
		/**
		 * \brief Find out how many CCDs the camera has
	 	 */
		int	nCcds();
		/**
		 * \brief Get Information about the CCD.
		 *
		 * This method does not return all available information, but
		 * typically only information available without accessing
		 * the CCD. It should be sufficient to plan an exposure.
		 */
		CcdInfo	getCcdinfo(int ccdid) throws NotFound, DeviceException;
		/**
		 * \brief Retrieve a CCD.
		 */
		Ccd*	getCcd(int ccdid) throws NotFound, DeviceException;
		// FilterWheel
		/**
		 * \brief Find out whether the camera has FilterWheel
		 */
		bool	hasFilterWheel();
		/**
		 * \brief Get the FilterWheel
		 */
		FilterWheel*	getFilterWheel() throws NotImplemented, DeviceException;
		// Guider Port
		/**
		 * \brief Find out whether the camera has a guider port.
	 	 */
		bool	hasGuidePort();
		/**
		 * \brief Get the Guider Port
		 */
		GuidePort*	getGuidePort() throws NotImplemented, DeviceException;
	};

	interface AdaptiveOpticsCallback extends Callback {
		void	point(Point p);
	};

	/**
	 * \brief AdaptiveOptics abstraction
	 */
	interface AdaptiveOptics extends Device {
		/**
 		 * \brief Set a position offset
		 */
		void	set(Point position);
		/**
		 * \brief Get the position offset
		 */
		Point	get();
		/**
		 * \brief Center (reset) the adaptive optics unit
		 */
		void	center();
		/**
		 * \brief find out whether the 
		 */
		bool	hasGuidePort();
		/**
		 * \brief get the guider port of the adaptive optics unit
		 */
		GuidePort*	getGuidePort() throws NotImplemented;

		void	registerCallback(Ice::Identity callback);
		void	unregisterCallback(Ice::Identity callback);
	};
};
