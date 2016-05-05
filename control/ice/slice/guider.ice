//
// guider.ice -- Interface to an autonomous guider
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil 2013
//
#include <camera.ice>
#include <Ice/Identity.ice>

module snowstar {
	/**
	 * \brief guiders are described by an Instrument name and indices
	 *
	 * A guider is defined by the instrument name and the indices of the
	 * guider ccd, guider port and adaptive optics unit.
	 */
	struct GuiderDescriptor {
		string	instrumentname;
		int	ccdIndex;
		int	guiderportIndex;
		int	adaptiveopticsIndex;
	};

	/**
	 * \brief Control device type selection 
	 *
	 * The guider can use guider ports or tip-tilt adaptive optics units.
	 * This means that when requesting calibration, we have to be able
	 * to distinguish the two, by using the ControlType enumeration.
	 */
	enum ControlType {
		ControlGuiderPort,
		ControlAdaptiveOptics
	};

	/**
	 * \brief Information about recent tracking activities
	 *
	 * A Tracking point contains information about when the point
	 * was measured, the offset that was measured and the correction
	 * that was applied.
	 */
	struct TrackingPoint {
		double	timeago;
		Point	trackingoffset;
		Point	activation;
		ControlType	type;
	};
	sequence<TrackingPoint> TrackingPoints;

	/**
	 * \brief Tracking History 
	 *
	 * The tracking history contains information about the guider that
	 * performed the tracking, the points encountered during the tracking
	 * and the time when the trackint started.
	 */
	struct TrackingHistory {
		int	guiderunid;
		double	timeago;
		int	guiderportcalid;
		int	adaptiveopticscalid;
		GuiderDescriptor	guider;
		TrackingPoints	points;
	};

	/**
	 * \brief A summary of the tracking
	 *
	 * The tracking 
	 */
	struct TrackingSummary {
		int	guiderunid;
		double	since;
		int	guiderportcalid;
		int	adaptiveopticscalid;
		GuiderDescriptor	guider;
		Point	lastoffset;
		Point	averageoffset;
		Point	variance;
	};

	/**
	 * \brief Interface to a tracking monitor
	 *
	 * A tracking monitor processes new points
	 */
	interface TrackingMonitor extends Callback {
		void	update(TrackingPoint ti);
	};

	/**
	 * \brief Calibration point structure
	 *
	 * A calibration point records the time offset after the start
	 * of the calibration process, the RA/DEC offset of the point and
	 * the position of the start resulting from applying the offset.
	 */
	struct CalibrationPoint {
		double	t;
		Point	offset;
		Point	star;
	};
	sequence<CalibrationPoint>	CalibrationSequence;

	/**
	 * \brief Calibration object
	 *
	 * The calibration contains information about the guider that was
	 * calibrated, the time when the calibration started, the 
	 * calibration coefficients found, and the points used to compute
	 * the calibration coefficients.
	 */
	sequence<float>	calibrationcoefficients;
	struct Calibration {
		int	id;
		double	timeago;
		GuiderDescriptor	guider;
		calibrationcoefficients	coefficients;
		double	quality;
		double	det;
		bool	complete;
		double	focallength;
		double	masPerPixel;
		ControlType	type;
		bool	flipped;
		CalibrationSequence	points;
	};

	/**
	 * \brief Interface for calibration point updates
	 *
	 * The calibration monitor processes updates for new calibration
	 * points.
	 */
	interface CalibrationMonitor extends Callback {
		void	update(CalibrationPoint point);
	};

	/**
	 * \brief States of the guider
	 */
	enum GuiderState {
		// Without the camera, ccd and guiderport selected,
		// the guider is not configured
		GuiderUNCONFIGURED,
		// When the camera, ccd and guider port have been
		// configured, the 
		GuiderIDLE,
		// Before the guider can be used for guiding, it must
		// be calibrated
		GuiderCALIBRATING,
		// When the guider ist calibrated, it can be used
		// for guiding
		GuiderCALIBRATED,
		// The calibrated guider can be used for guiding
		GuiderGUIDING
	};

	enum TrackerMethod {
		TrackerUNDEFINED,
		TrackerNULL,
		TrackerSTAR,
		TrackerPHASE,
		TrackerDIFFPHASE,
		TrackerLAPLACE,
		TrackerLARGE
	};
	/**
	 * \brief Interface for guiders
	 *
	 * Guiders take information from images shot through a ccd of a
	 * attached camera, and derive corrective actions that they then
	 * output to the guiderport.
	 */
	interface Guider {
		GuiderState	getState();

		/**
		 * \brief Get the CCD that was chosen for the guider
		 */
		Ccd*	getCcd() throws BadState;
		
		/**
		 * \brief Choose a guider port for the guider
		 *
		 * If a string of length 0 is given as the name, then
		 * it is assumed that the guider port of the chosen
		 * camera should be used
		 */
		GuiderPort*	getGuiderPort() throws BadState;

		/**
		 * \brief return the descriptor that created the guider
		 */
		GuiderDescriptor	getDescriptor();

		// The guider needs to know how to expose an image, where
		// to look for the guide star and where to lock it.
		void	setExposure(Exposure expo)
				throws BadParameter, BadState;
		Exposure	getExposure();

		// set/get the tracker method
		void	setTrackerMethod(TrackerMethod method);
		TrackerMethod	getTrackerMethod();

		// This is the position of the star we want to track.
		// It does not have to be exact at the beginning, and
		// the position is only used as a reference point during 
		// calibration. During guiding, the guide star is kept
		// in this position.
		void	setStar(Point star)
				throws BadParameter, BadState;
		Point	getStar() throws BadState;

		// if the repository name is set, then all images sent to the
		// callback will be added to the repository
		void	setRepositoryName(string reponame) throws NotFound;
		string	getRepositoryName();

		/**
		 * \brief Methods related to calibration
		 */
		void	useCalibration(int id, bool flipped)
					throws BadState, NotFound;
		Calibration	getCalibration(ControlType caltype)
					throws BadState;
		void	flipCalibration(ControlType caltype) throws BadState;
		void	unCalibrate(ControlType type) throws BadState;

		// methods to perform a calibration asynchronously
		int	startCalibration(ControlType caltype)
					throws BadState;
		double	calibrationProgress() throws BadState;
		void	cancelCalibration() throws BadState;
		bool	waitCalibration(double timeout) throws BadState;

		// methods for monitoring of the calibration process
		void	registerCalibrationMonitor(Ice::Identity calibrationmonitor);
		void	unregisterCalibrationMonitor(Ice::Identity calibrationmonitor);

		// Start and stop the guding process.
		// Before this can be done, the exposure parameters must be
		// specified, as the determine which are of the CCD to read
		// out and where to look for the star and where to lock it
		void	startGuiding(float gpinterval, float aointerval,
				bool stepping) throws BadState;
		float	getGuidingInterval() throws BadState;
		void	stopGuiding() throws BadState;

		// The following methods are used to monitor the calibration
		// or the guiding. The guider keeps the most recent image
		// so that a GUI application can fetch thosei mages and 
		// display them to the user
		/**
		 * \brief 
		 */
		Image*	mostRecentImage() throws BadState;

		/**
		 * \brief Polling interface to tracking information
		 */
		TrackingPoint	mostRecentTrackingPoint() throws BadState;

		/**
		 * \brief Get the complete tracking history
		 *
		 * \param guiderunid	ID of the guide run for which the
		 *			history is requested. -1 means the
		 *			currently active guide run.
		 */
		TrackingHistory	getTrackingHistory(int guiderunid)
						throws BadState;
		TrackingHistory	getTrackingHistoryType(int guiderunid,
			ControlType type) throws BadState;

		/**
		 * \brief get some statistics information about tracking
		 */
		TrackingSummary	getTrackingSummary() throws BadState;

		// monitor for tracking points
		void	registerTrackingMonitor(Ice::Identity trackingmonitor);
		void	unregisterTrackingMonitor(Ice::Identity trackingmonitor);

		// callbacks for images during guiding _and_ calibration
		void	registerImageMonitor(Ice::Identity imagemonitor);
		void	unregisterImageMonitor(Ice::Identity imagemonitor);
	};

	/**
	 * \brief The GuiderFactory builds Guider objects
	 *
	 * Guiders are persistent in the server, because the may have to be
	 * doing their work even when no client is connected. The GuiderFactory
	 * keeps a repository of guider objects. New guider objects can
	 * be created by specifying camera, ccd and guider port.
	 *
	 * In addition, the guider factory also gives access to calibration
	 * and tracking data. This is necessary because a guider may
	 * no longer be around when we want to retrieve the calibration
	 * or the tracking data. The camera may no longer be connected
	 * to the server, but the data should still be accessible.
	 */
	sequence<GuiderDescriptor> GuiderList;
	interface GuiderFactory {
		GuiderList	list();

		/**
		 * \brief Get a guider based on a guider descriptor
		 */
		Guider*	get(GuiderDescriptor descriptor) throws NotFound;


		/**
		 * \brief Retrieve a list of all valid calibration ids
		 */
		idlist	getAllCalibrations();

		/**
		 * \brief Retrieve a list of valid calibration ids for a guider
		 */
		idlist	getCalibrations(GuiderDescriptor guider);

		/**
		 * \brief Retrieve the Calibration by id
		 */
		Calibration	getCalibration(int id) throws NotFound;

		/**
		 * \brief Remove a calibration by id
		 */
		void	deleteCalibration(int id) throws NotFound;

		/**
		 * \brief Retrieve a list of all valid guide run ids
		 */
		idlist	getAllGuideruns();

		/**
		 * \brief Retrieve a list of valid guide run ids for a guider
		 */
		idlist	getGuideruns(GuiderDescriptor guider);

		/**
		 * \brief Retrieve the Tracking history by id
		 */
		TrackingHistory	getTrackingHistory(int id) throws NotFound;
		TrackingHistory	getTrackingHistoryType(int id,
			ControlType type) throws NotFound;

		/**
		 * \brief Remove a tracking history
		 */
		void	deleteTrackingHistory(int id) throws NotFound;
	};
};
