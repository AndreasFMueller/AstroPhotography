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
#include <AstroTypes.h>
#include <vector>
#include <set>
#include <cstdint>

namespace astro {
namespace camera {

/**
 * \brief Shutter class
 *
 * The shutter class is essentially empty except for a state type used
 * to indicate the current state of the shutter of a camera and conversion
 * functions from and to strings.
 */
class Shutter {
public:
	typedef enum state { CLOSED, OPEN } state;
static std::string	state2string(state s);
static state	string2state(const std::string& s);
};

/**
 * \brief Binning mode specification
 *
 * many astrophotography cameras support binning, i.e. combining adjacent pixels
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
	Binning(const std::string& binning);
	bool	operator==(const Binning& other) const;
	bool	operator!=(const Binning& other) const;
	bool	operator<(const Binning& other) const;
	unsigned int	getX() const { return x; }
	unsigned int	getY() const { return y; }
	void	setX(unsigned int _x) { x = _x; }
	void	setY(unsigned int _y) { y = _y; }
	virtual std::string	toString() const;
};
std::ostream&	operator<<(std::ostream& out, const Binning& binning);
std::istream&	operator>>(std::istream& out, Binning& binning);

astro::image::ImagePoint	operator*(const astro::image::ImagePoint& point,
					const Binning& mode);
astro::image::ImagePoint	operator/(const astro::image::ImagePoint& point,
					const Binning& mode);
astro::image::ImageSize	operator*(const astro::image::ImageSize& size,
				const Binning& mode);
astro::image::ImageSize	operator/(const astro::image::ImageSize& size,
				const Binning& mode);

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
 *
 * The frame rectangle is specified in unbinned pixels. So a 2x3 binned
 * subframe with size (200,300) results in a 100x100 image being returned.
 */
class	Exposure {
public:
	astro::image::ImageRectangle	frame;
	float	exposuretime;
	float	gain;
	float	limit;
	Binning	mode;
	Shutter::state	shutter;

	// fields related tot he exposure purpose
	typedef	enum { light = 0, dark = 1, flat = 2 } purpose_t;
	purpose_t	purpose;
static std::string	purpose2string(purpose_t p);
static purpose_t	string2purpose(const std::string& p);

	Exposure();
	Exposure(const astro::image::ImageRectangle& _frame,
		float _exposuretime = 1);

	// state related methods
	typedef enum state_e {
		idle, exposing, exposed, cancelling
	} State;
static std::string	state2string(State s);
static State	string2state(const std::string& s);
	virtual std::string	toString() const;

	void	addToImage(astro::image::ImageBase& image) const;
};
std::ostream&	operator<<(std::ostream& out, const Exposure& exposure);

// We will need a number of shared_ptr classes for different types of devices

class FilterWheel;
typedef std::shared_ptr<FilterWheel>	FilterWheelPtr;

class GuiderPort;
typedef std::shared_ptr<GuiderPort>	GuiderPortPtr;

class Cooler;
typedef std::shared_ptr<Cooler>	CoolerPtr;

class Focuser;
typedef std::shared_ptr<Focuser>	FocuserPtr;

class Camera;
typedef std::shared_ptr<Camera>	CameraPtr;

class Ccd;
typedef std::shared_ptr<Ccd>	CcdPtr;

/**
 * \brief Class containing information about a CCD chip. 
 *
 * This class tracks commonly used information about a CCD chip inside
 * a camera. The camera class has methods to return information about
 * the CCDs without a need to open the CCD. Instances of this class can
 * easily be copied, not as the Ccd instances.
 */
class CcdInfo {
	// CCD name
	DeviceName	_name;
public:
	static DeviceName	defaultname(const DeviceName& parent,
					const std::string& unitname);
	const DeviceName&	name() const { return _name; }
	// CCD size
private:
	astro::image::ImageSize	_size;
public:
	const astro::image::ImageSize&	size() const { return _size; }
	const astro::image::ImageRectangle	getFrame() const;
	// CCD id
private:
	int	ccdid;
public:
	int	getId() const { return ccdid; }
	// binning modes
private:
	BinningSet	binningmodes;
public:
	const BinningSet&	modes() const { return binningmodes; }
	void	addMode(const Binning& mode);
	void	addModes(const BinningSet& modes);
	// shutter related methods
private:
	bool	_shutter;
public:
	void	setShutter(bool shutter) { _shutter = shutter; }
	bool	shutter() const { return _shutter; }
private:
	float	_pixelwidth;	// pixel width in meters
	float	_pixelheight;	// pixel height in meters
public:
	float	pixelwidth() const { return _pixelwidth; }
	float	pixelheight() const { return _pixelheight; }
	void	pixelwidth(float pixelwidth) { _pixelwidth = pixelwidth; }
	void	pixelheight(float pixelheight) { _pixelheight = pixelheight; }
public:
	CcdInfo(const std::string& name, const astro::image::ImageSize& size,
		int ccdid = 0);
	CcdInfo(const CcdInfo& other);
	CcdInfo&	operator=(const CcdInfo& other);

	// text representation
	virtual std::string	toString(bool withbinningmodes = false) const;

	// utility functions
	astro::image::ImageRectangle	clipRectangle(const astro::image::ImageRectangle& rectangle) const;
	astro::image::ImageRectangle	centeredRectangle(const astro::image::ImageSize& size) const;

	// add metadata from the CCDinfo
	void	addMetadata(ImageBase& image) const;
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
class	Ccd : public astro::device::Device {
protected:
	CcdInfo	info;
	volatile Exposure::State	state;
	float		setTemperature;
	Exposure	exposure;
	void	addBinning(const Binning& binning);
	time_t	lastexposurestart;
public:
	typedef CcdPtr	sharedptr;
	static DeviceName::device_type	devicetype;
	static DeviceName	defaultname(const DeviceName& parent,
					const std::string& unitname);
	Ccd(const CcdInfo& _info)
		: astro::device::Device(_info.name(), DeviceName::Ccd),
		info(_info), state(Exposure::idle) { }
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
	virtual Shutter::state	getShutterState();
	virtual void	setShutterState(const Shutter::state& state);

	// gain related methods
	virtual bool	hasGain() { return false; }
	virtual std::pair<float, float>	gainInterval() {
		return std::make_pair((float)0, (float)0);
	}

	// image retrievel functions
private:
	virtual astro::image::ImagePtr	getRawImage();
public:
	astro::image::ImagePtr	getImage();
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

/**
 * \brief Abstraction for a astrophotographic CCD camera.
 *
 * A camera can have several CCDs, which are numbered sequentially starting
 * at 0. 
 */
class	Camera : public astro::device::Device {
protected:
	std::vector<CcdInfo>	ccdinfo;
public:
	typedef CameraPtr	sharedptr;
	static DeviceName::device_type	devicetype;
	static DeviceName	defaultname(const DeviceName& parent,
					const std::string&unitname);
	Camera(const std::string& name);
	Camera(const DeviceName& name);
	virtual	~Camera();
	virtual void	reset();
	unsigned int	nCcds() const;
	const CcdInfo&	getCcdInfo(size_t ccdid) const;
private:
	std::vector<CcdPtr>	ccds;
protected:
	virtual CcdPtr	getCcd0(size_t ccdid) = 0;
public:
	CcdPtr	getCcd(size_t ccdid);
	CcdPtr	getCcd(const DeviceName& ccdname);

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
class Cooler : public astro::device::Device {
protected:
	float	temperature;
public:
	typedef CoolerPtr	sharedptr;
	static DeviceName::device_type	devicetype;
	static DeviceName	defaultname(const DeviceName& parent,
					const std::string& unitname);
	Cooler(const DeviceName& name);
	Cooler(const std::string& name);
	virtual ~Cooler();
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
	virtual void	addTemperatureMetadata(astro::image::ImageBase& image);
	bool	stable();
	bool	wait(float timeout);
};

/**
 * \brief FilterWheel abstraction
 *
 * Some Cameras have an included filter wheel, while in other cases
 * the filter wheel is controlled by a different driver
 */
class FilterWheel : public astro::device::Device {
public:
	typedef enum state_e { idle, moving, unknown } State;
static std::string	state2string(State s);
static State	string2state(const std::string& s);
	typedef FilterWheelPtr	sharedptr;
	static DeviceName::device_type	devicetype;
	static DeviceName	defaultname(const DeviceName& parent,
					const std::string& unitname);
	FilterWheel(const std::string& name);
	FilterWheel(const DeviceName& name);
	virtual ~FilterWheel();
	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition() = 0;
	virtual void	select(size_t filterindex) = 0;
	virtual void	select(const std::string& name);
	virtual std::string	filterName(size_t filterindex);
	virtual State	getState() = 0;
	bool	wait(float timeout);
};

/**
 * \brief Abstraction for the Guider port
 */
class GuiderPort : public astro::device::Device {
public:
	typedef GuiderPortPtr	sharedptr;
	static DeviceName::device_type	devicetype;
	static DeviceName	defaultname(const DeviceName& parent,
					const std::string& unitname);
	GuiderPort(const std::string& name);
	GuiderPort(const DeviceName& name);
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

/**
 * \brief Abstraction for a Focuser
 */
class Focuser : public astro::device::Device {
public:
	typedef FocuserPtr	sharedptr;
	static DeviceName::device_type	devicetype;
	static DeviceName	defaultname(const DeviceName& parent,
					const std::string& unitname);
	Focuser(const DeviceName& name);
	Focuser(const std::string& name);
	virtual ~Focuser();
	virtual unsigned short	min();
	virtual unsigned short	max();
	virtual unsigned short	current();
	virtual unsigned short	backlash();
	virtual void	set(unsigned short value);
	bool	moveto(unsigned short value, unsigned long timeout = 60);
};

class AdaptiveOptics;
typedef std::shared_ptr<AdaptiveOptics>	AdaptiveOpticsPtr;

/**
 * \brief Adaptive Optics unit
 */

class AdaptiveOptics : public astro::device::Device {
protected:
	bool	_hasguiderport;
public:
	typedef AdaptiveOpticsPtr	sharedptr;
	static DeviceName::device_type	devicetype;
	static DeviceName	defaultname(const DeviceName& parent,
					const std::string& unitname);
	AdaptiveOptics(const DeviceName& name);
	AdaptiveOptics(const std::string& name);
	virtual ~AdaptiveOptics();
	// change the position of the adaptive optics device
	Point	currentposition;
protected:
	virtual void	set0(const Point& position);
public:
	void	set(const Point& position);
	Point	get() const { return currentposition; }
	void	center();
	// interface to the guider port contained in the unit
	bool	hasGuiderPort() const { return _hasguiderport; }
protected:
	virtual GuiderPortPtr	getGuiderPort0();
public:
	GuiderPortPtr	getGuiderPort();
};

/**
 * \brief Adapter template to extract an unspecified device from a camera
 */
template<typename device>
class CameraDeviceAdapter {
	CameraPtr	_camera;
public:
	CameraDeviceAdapter(CameraPtr camera) : _camera(camera) { }
	typename device::sharedptr	get(const DeviceName& name);
};

template<>
CcdPtr	CameraDeviceAdapter<Ccd>::get(const DeviceName& name);

template<>
GuiderPortPtr	CameraDeviceAdapter<GuiderPort>::get(const DeviceName& name);

template<>
FilterWheelPtr	CameraDeviceAdapter<FilterWheel>::get(const DeviceName& name);

} // namepsace camera
} // namespace astro

#endif /* _AstroCamera_h */
