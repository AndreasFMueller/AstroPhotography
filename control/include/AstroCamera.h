/*
 * AstroCamera.h -- Astro camera declarations
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroCamera_h
#define _AstroCamera_h

#include <AstroImage.h>
#include <AstroDevice.h>
#include <vector>
#include <set>
#include <stdint.h>

namespace astro {
namespace camera {

typedef enum shutter_state { SHUTTER_CLOSED, SHUTTER_OPEN } shutter_state;

/**
 * \brief Binning mode specification
 *
 * many astrophotography cameras support, i.e. combining adjacent pixels
 * to form larger pixels. The coordinate values in a binning object
 * can also be set to -1, which means that any positive number would
 * be valid. This simplifies specifying the acceptable binning modes as a list
 * Binning objects.
 */
class	Binning {
	unsigned int	x;
	unsigned int	y;
public:
	Binning(unsigned int _x = 1, unsigned int _y = 1);
	Binning(const Binning& other) : x(other.x), y(other.y) { }
	bool	operator==(const Binning& other) const;
	bool	operator<(const Binning& other) const;
	unsigned int	getX() const { return x; }
	unsigned int	getY() const { return y; }
	virtual std::string	toString() const;
};
std::ostream&	operator<<(std::ostream& out, const Binning& binning);

/**
 * \brief Set of Binning objects
 *
 * Usually a camera or rather a CCD chip will allow a certain set of
 * binning modes as specified by a BinningSet object. The permits
 * method can then be used to determine whether a proposed binning
 * mode is supported by the chip. This cannot be a set membership test
 * because wildcard binning modes.
 */
class	BinningSet : public std::set<Binning> {
public:
	BinningSet();
	bool	permits(const Binning& binning) const throw (std::range_error);
	virtual std::string	toString() const;
};
std::ostream&	operator<<(std::ostream& out, const BinningSet& binningset);

/**
 * \brief Specification of an exposure request
 *
 * The Exposure object specifies an exposure request to a camera.
 * It includes the subframe to expose, the exposure time, the
 * the binning mode and the gain.
 */
class	Exposure {
public:
	astro::image::ImageRectangle	frame;
	float	exposuretime;
	float	gain;
	float	limit;
	Binning	mode;
	shutter_state	shutter;

	Exposure();
	Exposure(const astro::image::ImageRectangle& _frame,
		float _exposuretime);
		
	typedef enum state_e {
		idle, exposing, exposed, cancelling
	} State;
	virtual std::string	toString() const;

	void	addToImage(astro::image::ImageBase& image) const;
};
std::ostream&	operator<<(std::ostream& out, const Exposure& exposure);

/**
 * \brief Class containing information about a CCD chip. 
 *
 * This class tracks commonly used information about a CCD chip inside
 * a camera. The camera class has methods to return information about
 * the CCDs without a need to open the CCD. Instances of this class can
 * easily be copied, not as the Ccd instances.
 */
class Camera;
class Ccd;
class CcdInfo {
	std::string	_name;
	astro::image::ImageSize	_size;
	int	ccdid;
	BinningSet	binningmodes;
	bool	_shutter;
public:
	CcdInfo(const std::string& name, const astro::image::ImageSize& size,
		int ccdid = 0);

	// modifying accessors
	void	addMode(const Binning& mode);
	void	setShutter(bool shutter) { _shutter = shutter; }
	bool	shutter() const { return _shutter; }

	// accessors
	const astro::image::ImageSize&	size() const;
	const astro::image::ImageRectangle	getFrame() const;
	const BinningSet&	modes() const;
	const std::string&	name() const;
	int	getId() const;

	// text representation
	virtual std::string	toString() const;

	// utility functions
	astro::image::ImageRectangle	clipRectangle(const astro::image::ImageRectangle& rectangle) const;
	astro::image::ImageRectangle	centeredRectangle(const astro::image::ImageSize& size) const;
};
std::ostream&	operator<<(std::ostream& out, const CcdInfo& ccdinfo);

/**
 * \brief Abstraction for a CCD chip
 *
 * This class is necessary because a camera can have several imaging
 * chips. All interactions during taking an exposure happen only through
 * an instance of the Ccd class. The base class just provides an interface,
 * each camera module must implemented derived classes that override
 * those functions that the camera can implement.
 *
 * A camera should only implement those image retrieval functions that
 * the camera can return natively. For all other pixel formats, the
 * application should use the provided conversion functions.
 */
class Cooler;
typedef std::tr1::shared_ptr<Cooler>	CoolerPtr;

class	Ccd {
protected:
	CcdInfo	info;
	volatile Exposure::State	state;
	float		setTemperature;
	Exposure	exposure;
	void	addBinning(const Binning& binning);
	time_t	lastexposurestart;
public:
	Ccd(const CcdInfo& _info) : info(_info), state(Exposure::idle) { }
	virtual	~Ccd() { }
	const CcdInfo&	getInfo() const { return info; }
	const astro::image::ImageSize&	getSize() const { return info.size(); }

	// methods to start/stop exposures
	virtual void	startExposure(const Exposure& exposure);
	virtual Exposure::State	exposureStatus();
	virtual void	cancelExposure();
	const Exposure&	getExposure() const { return exposure; }
	virtual bool	wait();

	// methods to control a shutter
	bool	hasShutter() const { return info.shutter(); }
	virtual shutter_state	getShutterState();
	virtual void	setShutterState(const shutter_state& state);

	// gain related methods
	virtual bool	hasGain() { return false; }
	virtual std::pair<float, float>	gainInterval() {
		return std::make_pair((float)0, (float)0);
	}

	// image retrievel functions
	virtual astro::image::ImagePtr	getImage();
	virtual astro::image::ImageSequence	getImageSequence(unsigned int imagecount);

	// handling the cooler
private:
	CoolerPtr	cooler;
protected:
	virtual CoolerPtr	getCooler0();
public:
	virtual bool	hasCooler() const { return false; }
	CoolerPtr	getCooler();

	// methods related to metadata
	virtual void	addExposureMetadata(astro::image::ImageBase& image) const;
	virtual void	addTemperatureMetadata(astro::image::ImageBase& image);
	virtual void	addMetadata(astro::image::ImageBase& image);
};
typedef std::tr1::shared_ptr<Ccd>	CcdPtr;

/**
 * \brief Abstraction for a astrophotographic CCD camera.
 *
 * A camera can have several CCDs, which are numbered sequentially starting
 * at 0. 
 */
class FilterWheel;
typedef std::tr1::shared_ptr<FilterWheel>	FilterWheelPtr;

class GuiderPort;
typedef std::tr1::shared_ptr<GuiderPort>	GuiderPortPtr;

class	Camera : public astro::device::Device {
protected:
	std::vector<CcdInfo>	ccdinfo;
public:
	Camera(const std::string& name);
	Camera();
	~Camera();
	virtual void	reset();
	unsigned int	nCcds() const;
	const CcdInfo&	getCcdInfo(size_t ccdid) const;
private:
	std::vector<CcdPtr>	ccds;
protected:
	virtual CcdPtr	getCcd0(size_t ccdid) = 0;
public:
	CcdPtr	getCcd(size_t ccdid);

	// handling the filter wheel
private:
	FilterWheelPtr	filterwheel;
protected:
	virtual FilterWheelPtr	getFilterWheel0();
public:
	virtual bool	hasFilterWheel() const { return false; }
	FilterWheelPtr	getFilterWheel();

	// handling the guider port
private:
	GuiderPortPtr	guiderport;
protected:
	virtual GuiderPortPtr	getGuiderPort0();
public:
	virtual bool	hasGuiderPort() const { return false; }
	GuiderPortPtr	getGuiderPort();
};
typedef std::tr1::shared_ptr<Camera>	CameraPtr;

/**
 * \brief Cooler abstraction
 *
 * Some CCDs of some cameras have a thermoelectric cooler that helps reduce
 * chip temperature and noise. This class abstracts what such coolers can do
 * to the bare minimum: query and set the temperature. The temperatures
 * are always in K. This may look awkward, but the philosophy everywhere in
 * this projects is to use SI units. It is better to have a general rule
 * regarding units than to have to document units for every value.
 */
class Cooler {
protected:
	float	temperature;
public:
	Cooler();
	virtual ~Cooler();
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
	virtual void	addTemperatureMetadata(astro::image::ImageBase& image);
};

/**
 * \brief FilterWheel abstraction
 *
 * Some Cameras have an included filter wheel, while in other cases
 * the filter wheel is controlled by a different driver
 */
class FilterWheel : public astro::device::Device {
public:
	FilterWheel();
	virtual ~FilterWheel();
	virtual unsigned int	nFilters() = 0;
	virtual unsigned int	currentPosition() = 0;
	virtual void	select(size_t filterindex) = 0;
	virtual std::string	filterName(size_t filterindex) = 0;
};

/**
 * \brief Abstraction for the Guider port
 */
class GuiderPort : public astro::device::Device {
public:
	GuiderPort();
	virtual ~GuiderPort();

	typedef enum {
		DECMINUS = 1, DECPLUS = 2, RAMINUS = 4, RAPLUS = 8
	} relay_bits;

	/**
 	 * \brief Find out which guider port relays are active
	 * 
	 * The return value of this method is a bit map composed from the
	 * constants in the relay_bits enum.
	 */
	virtual uint8_t	active() = 0;

	/**
	 * \brief Activate guider port relays.
	 *
	 * \param raplus	activation time for RA+ relay
	 * \param raminus	activation time for RA- relay
	 * \param decplus	activation time for DEC+ relay
	 * \param decminus	activation time for DEC- relay
	 */
	virtual void	activate(float raplus, float raminus,
		float decplus, float decminus) = 0;
};

} // namepsace camera
} // namespace astro

#endif /* _AstroCamera_h */
