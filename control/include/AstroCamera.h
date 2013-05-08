/*
 * AstroCamera.h
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroCamera_h
#define _AstroCamera_h

#include <AstroImage.h>
#include <vector>

namespace astro {
namespace camera {

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
	int	x;
	int	y;
public:
	Binning(int _x = 1, int _y = 1) : x(_x), y(_y) { }
	Binning(const Binning& other) : x(other.x), y(other.y) { }
	bool	iswildcard() const;
	bool	operator==(const Binning& other) const;
	bool	compatible(const Binning& other) const;
	int	getX() const { return x; }
	int	getY() const { return y; }
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
class	BinningSet : public std::vector<Binning> {
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

	Exposure();
	Exposure(const astro::image::ImageRectangle& _frame,
		float _exposuretime);
		
	typedef enum state_e {
		idle, exposing, exposed, cancelling
	} State;
	virtual std::string	toString() const;
};
std::ostream&	operator<<(std::ostream& out, const Exposure& exposure);

/**
 * \brief Exception for unimplementd stuff
 *
 * Rather than not implementing some of the functions, and thus creating
 * interface classes with many pure virtual functions which the user
 * must implement before it can at all be instantiated, we prefer 
 * the more dynamical approach of implementing all with a function that
 * throws a special exception designed to indicate that the function is
 * not implemented.
 */
class not_implemented : public std::runtime_error {
public:
	not_implemented(const std::string& cause) : std::runtime_error(cause) {}
};

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
public:
	astro::image::ImageSize	size;
	BinningSet	binningmodes;
	std::string	name;
	int	ccdid;
	CcdInfo();
	const astro::image::ImageSize&	getSize() const;
	const BinningSet&	modes() const;
	const std::string&	getName() const;
	int	getId() const;
	friend class Camera;
	friend class Ccd;
	virtual std::string	toString() const;
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
	Exposure::State	state;
	float		setTemperature;
	Exposure	exposure;
	void	addBinning(const Binning& binning);
public:
	Ccd(const CcdInfo& _info) : info(_info), state(Exposure::idle) { }
	virtual	~Ccd() { }
	const CcdInfo&	getInfo() const { return info; }
	const astro::image::ImageSize&	getSize() const { return info.size; }

	// methods to start/stop exposures
	virtual void	startExposure(const Exposure& exposure)
		throw (not_implemented);
	virtual Exposure::State	exposureStatus() throw (not_implemented);
	virtual void	cancelExposure() throw (not_implemented);
	const Exposure&	getExposure() const { return exposure; }

	// image retrievel functions
	virtual astro::image::ByteImagePtr	byteImage()
		throw (not_implemented);
	virtual astro::image::ShortImagePtr	shortImage()
		throw (not_implemented);
	virtual astro::image::IntImagePtr	intImage()
		throw (not_implemented);
	virtual astro::image::LongImagePtr	longImage()
		throw (not_implemented);
	virtual astro::image::YUYVImagePtr	yuyvImage()
		throw (not_implemented);
	virtual astro::image::RGBImagePtr	rgbImage()
		throw (not_implemented);

	// handling the cooler
	virtual CoolerPtr	getCooler() throw (not_implemented);
};
typedef std::tr1::shared_ptr<Ccd>	CcdPtr;

/**
 * \brief Abstraction for a astrophotographic CCD camera.
 *
 * A camera can have several CCDs, which are numbered sequentially starting
 * at 0. 
 */
class	Camera {
protected:
	std::vector<CcdInfo>	ccdinfo;
public:
	Camera() { }
	~Camera() { }
	int	nCcds() const { return ccdinfo.size(); }
	const CcdInfo&	getCcdInfo(size_t ccdid) { return ccdinfo[ccdid]; }
	virtual CcdPtr	getCcd(int id) = 0;
};
typedef std::tr1::shared_ptr<Camera>	CameraPtr;

/**
 * \brief Coole abstraction
 *
 * Some CCDs of some cameras have a thermoelectric cooler that helps reduce
 * chip temperature and noise. This class abstracts what such coolers can do
 * to the bare minimum: query and set the temperature. The temperatures
 * are always in K. This may look awkward, but the philosophy everywhere in
 * this projects is to use SI units. It is better to have a general rule
 * regarding units than to have to document units for every value.
 */
class Cooler {
	float	temperature;
public:
	Cooler();
	virtual ~Cooler();
	virtual float	getSetTemperature();
	virtual float	getActualTemperature();
	virtual void	setTemperature(const float temperature);
	virtual bool	isOn();
	virtual void	setOn(bool onoff);
};

/**
 * \brief Camera locator
 *
 * The camera locator class acts as a factory for camera instances.
 * A module contains a single instance of the locator class. 
 */
class	CameraLocator {
public:
	CameraLocator() { }
	virtual	~CameraLocator() { }
	virtual std::string	getName() const = 0;
	virtual std::string	getVersion() const = 0;
	virtual std::vector<std::string>	getCameralist() = 0;
	virtual CameraPtr	getCamera(const std::string& name) = 0;
};
typedef std::tr1::shared_ptr<CameraLocator>	CameraLocatorPtr;

} // namepsace camera
} // namespace astro

#endif /* _AstroCamera_h */
