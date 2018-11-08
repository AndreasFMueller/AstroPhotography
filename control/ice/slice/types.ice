//
// types.ice -- common type definitions
//
// (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <Ice/Identity.ice>
#include <exceptions.ice>

/**
 * \brief snowstar module captures all interfaces
 */
module snowstar {
	sequence<int>	idlist;

	// Image related data structures
	/**
	 * \brief Pixel coordinates of a pixel in an image
	 *
	 * To be consistent with FITS, the origin of the coordinate system
	 * is in the lower left corner of the image.
 	 */
	struct ImagePoint {
		int	x;
		int	y;
	};

	/**
	 * \brief Size of an image in pixels
	 */
	struct ImageSize {
		int	width;
		int	height;
	};

	/**
	 * \brief Rectangle inside an image
	 */
	struct ImageRectangle {
		ImagePoint	origin;
		ImageSize	size;
	};

	/**
	 * \brief generic point
	 *
	 * This type is used for guiding, where subpixel resolution is
	 * required, which leads to using floating coordinates
	 */
	struct Point {
		float	x;
		float	y;
	};

	/**
	 * \brief sky point specification for equatorial mounts
	 */
	struct RaDec {
		float	ra;	// right ascension in hours
		float	dec;	// declination in degrees
	};

	/**
	 * \brief sky point specification for AltAzimut mounts
	 */
	struct AzmAlt {
		float	azm;	// azimut in degrees
		float	alt;	// altitude above horizon in degrees
	};

	/**
	 * \brief Longitude and latitude, use for GPS positions from mount
	 */
	struct LongLat {
		float	longitude;	// logitude in degrees
		float	latitude;	// latitude in degrees
	};

	/**
	 * \brief base class for all callbacks interfaces
	 *
	 * Callbacks monitor some process, so they all need notifcation when
	 * that process has completed. They may want to extend that facility
	 * e.g. to report the final state.
	 */
	interface Callback {
		void	stop();
	};

	/**
	 * \brief Configuration data structures
	 */
	struct ConfigurationKey {
		string	domain;
		string	section;
		string	name;
	};
	struct ConfigurationItem {
		string	domain;
		string	section;
		string	name;
		string	value;
	};
	sequence<ConfigurationItem>	ConfigurationList;

	/**
 	 * \brief Configuration interface
	 */
	interface Configuration {
		bool	has(ConfigurationKey key);
		ConfigurationItem	get(ConfigurationKey key)
						throws NotFound;
		void	set(ConfigurationItem item) throws BadParameter;
		void	remove(ConfigurationKey key) throws NotFound;
		ConfigurationList	list();
		ConfigurationList	listDomain(string domain);
		ConfigurationList	listSection(string domain,
						string section);
	};

	struct FileInfo {
		string	name;
		bool	writeable;
	};

	sequence<string>	FilenameList;
	struct DirectoryInfo {
		string	name;
		bool	writeable;
		FilenameList	files;
		FilenameList	directories;
	};

	/**
	 * \brief Interface to the server functions
	 *
	 * These relate to process control and more generally operating
	 * system functions.
	 */
	interface Daemon {
		void	reloadRepositories();
		void	shutdownServer(float delay);
		void	restartServer(float delay);
		void	shutdownSystem(float delay);
		FileInfo	statFile(string filename)
					throws NotFound, IOException;
		DirectoryInfo	statDirectory(string dirname)
					throws NotFound, IOException;
		FileInfo	statDevice(string filename)
					throws NotFound, IOException;
		void	mount(string device, string mountpoint)
				throws NotFound, IOException, OperationFailed;
		void	unmount(string mountpoint)
				throws NotFound, IOException, OperationFailed;
		long	getSystemTime();
		void	setSystemTime(long unixtime);
	};

	/**
	 * \brief Binning mode used for an image
	 */
	struct BinningMode {
		int	x;
		int	y;
	};

	/**
	 * \brief parameter value types
	 */
	enum ParameterType {
		ParameterBoolean,
		ParameterRange,
		ParameterSequence,
		ParameterSetFloat,
		ParameterSetString
	};

	sequence<float>	floatlist;
	sequence<string>	stringlist;

	/**
	 * \brief A parameter description structure
	 *
	 * some parameters are not needed, but we will hide them when we
	 * convert the object into an astro::device::ParameterDescription
	 * object.
	 */
	struct ParameterDescription {
		ParameterType	type;
		string	name;
		float	from;
		float	to;
		float	step;
		floatlist	floatvalues;
		stringlist	stringvalues;
	};

	/**
	 * \brief An object having parameters that can be set
	 */
	interface Device {
		string	getName();
		stringlist	parameterNames();
		bool	hasParameter(string name);
		ParameterDescription	parameter(string name);
		void	setParameterFloat(string name, double value);
		void	setParameterString(string name, string value);
		float	parameterValueFloat(string name);
		string	parameterValueString(string name);
	};

	enum EventLevel {
		EventLevelDEBUG,
		EventLevelINFO,
		EventLevelNOTICE,
		EventLevelWARNING,
		EventLevelERR,
		EventLevelCRIT,
		EventLevelALERT,
		EventLevelEMERG
	};

	/**
 	 * \brief An event record
	 */
	struct Event {
		int		id;
		EventLevel	level;
		int		pid;
		string		service;
		double		timeago;
		string		subsystem;
		string		message;
		string		classname;
		string		file;
		int		line;
	};
	sequence<Event>	eventlist;

	interface EventMonitor extends Callback {
		void	update(Event eventinfo);
	};

	interface EventHandler {
		Event		eventId(int id);
		eventlist	eventsBetween(double fromago, double toago);
		eventlist	eventsCondition(string condition);
		void	registerMonitor(Ice::Identity eventmonitor);
                void	unregisterMonitor(Ice::Identity eventmonitor);
	};

	/**
	 * \brief Gateway for status information
 	 */
	struct StatusUpdate {
		float	avgguideerror;
		float	ccdtemperature;
		double	lastimagestartago;
		float	exposuretime;
		int	currenttaskid;
		int	pendingtasks;
		float	ra;
		float	dec;
		float	hourangle;
		int	filter;
	};

	interface StatusUpdateMonitor extends Callback {
		void	update(StatusUpdate status);
	};

	interface Gateway {
		void	send(StatusUpdate update);
		void	registerMonitor(Ice::Identity statusupdatemonitor);
                void	unregisterMonitor(Ice::Identity statusupdatemonitor);
	};
};
