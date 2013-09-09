/*
 * Conversions.cpp -- type conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Conversions.h>
#include <AstroDebug.h>

namespace astro {

// Device type

Astro::DeviceLocator::device_type	convert(
	astro::device::DeviceLocator::device_type fromtype) {
	switch (fromtype) {
	case astro::device::DeviceLocator::CAMERA:
		return Astro::DeviceLocator::DEVICE_CAMERA;
	case astro::device::DeviceLocator::FOCUSER:
		return Astro::DeviceLocator::DEVICE_FOCUSER;
	case astro::device::DeviceLocator::GUIDERPORT:
		return Astro::DeviceLocator::DEVICE_GUIDERPORT;
	case astro::device::DeviceLocator::FILTERWHEEL:
		return Astro::DeviceLocator::DEVICE_FILTERWHEEL;
	case astro::device::DeviceLocator::COOLER:
		return Astro::DeviceLocator::DEVICE_COOLER;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromtype);
	throw std::runtime_error("illegal type");
}

astro::device::DeviceLocator::device_type	convert(
		Astro::DeviceLocator::device_type fromtype) {
	switch (fromtype) {
	case Astro::DeviceLocator::DEVICE_CAMERA:
		return astro::device::DeviceLocator::CAMERA;
	case Astro::DeviceLocator::DEVICE_FOCUSER:
		return astro::device::DeviceLocator::FOCUSER;
	case Astro::DeviceLocator::DEVICE_GUIDERPORT:
		return astro::device::DeviceLocator::GUIDERPORT;
	case Astro::DeviceLocator::DEVICE_FILTERWHEEL:
		return astro::device::DeviceLocator::FILTERWHEEL;
	case Astro::DeviceLocator::DEVICE_COOLER:
		return astro::device::DeviceLocator::COOLER;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromtype);
	throw std::runtime_error("illegal type");
}

std::string	convert2string(astro::device::DeviceLocator::device_type fromtype) {
	switch (fromtype) {
	case astro::device::DeviceLocator::CAMERA:
		return std::string("CAMERA");
	case astro::device::DeviceLocator::FOCUSER:
		return std::string("FOCUSER");
	case astro::device::DeviceLocator::GUIDERPORT:
		return std::string("GUIDERPORT");
	case astro::device::DeviceLocator::FILTERWHEEL:
		return std::string("FILTERWHEEL");
	case astro::device::DeviceLocator::COOLER:
		return std::string("COOLER");
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromtype);
	throw std::runtime_error("illegal type");
}

std::string	convert2string(Astro::DeviceLocator::device_type fromtype) {
	switch (fromtype) {
	case Astro::DeviceLocator::DEVICE_CAMERA:
		return std::string("CAMERA");
	case Astro::DeviceLocator::DEVICE_FOCUSER:
		return std::string("FOCUSER");
	case Astro::DeviceLocator::DEVICE_GUIDERPORT:
		return std::string("GUIDERPORT");
	case Astro::DeviceLocator::DEVICE_FILTERWHEEL:
		return std::string("FILTERWHEEL");
	case Astro::DeviceLocator::DEVICE_COOLER:
		return std::string("COOLER");
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromtype);
	throw std::runtime_error("illegal type");
}

// Exposure state

Astro::ExposureState	convert(astro::camera::Exposure::State fromstate) {
	switch (fromstate) {
	case astro::camera::Exposure::idle:
		return Astro::EXPOSURE_IDLE;
	case astro::camera::Exposure::exposing:
		return Astro::EXPOSURE_EXPOSING;
	case astro::camera::Exposure::exposed:
		return Astro::EXPOSURE_EXPOSED;
	case astro::camera::Exposure::cancelling:
		return Astro::EXPOSURE_CANCELLING;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromstate);
	throw std::runtime_error("illegal type");
}

astro::camera::Exposure::State	convert(Astro::ExposureState fromstate) {
	switch (fromstate) {
	case Astro::EXPOSURE_IDLE:
		return astro::camera::Exposure::idle;
	case Astro::EXPOSURE_EXPOSING:
		return astro::camera::Exposure::exposing;
	case Astro::EXPOSURE_EXPOSED:
		return astro::camera::Exposure::exposed;
	case Astro::EXPOSURE_CANCELLING:
		return astro::camera::Exposure::cancelling;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromstate);
	throw std::runtime_error("illegal type");
}

std::string	convert2string(astro::camera::Exposure::State fromstate) {
        switch (fromstate) {
        case astro::camera::Exposure::idle:
                return std::string("IDLE");
        case astro::camera::Exposure::exposing:
                return std::string("EXPOSING");
        case astro::camera::Exposure::exposed:
                return std::string("EXPOSED");
        case astro::camera::Exposure::cancelling:
                return std::string("CANCELLING");
        }
        debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromstate);
        throw std::runtime_error("illegal type");
}

std::string	convert2string(Astro::ExposureState fromstate) {
	switch (fromstate) {
	case Astro::EXPOSURE_IDLE:
		return std::string("idle");
	case Astro::EXPOSURE_EXPOSING:
		return std::string("exposing");
	case Astro::EXPOSURE_EXPOSED:
		return std::string("exposed");
	case Astro::EXPOSURE_CANCELLING:
		return std::string("cancelling");
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", fromstate);
	throw std::runtime_error("illegal type");
}


// Image point

astro::image::ImagePoint	convert(const Astro::ImagePoint& point) {
	return ImagePoint(point.x, point.y);
}

Astro::ImagePoint	convert(const astro::image::ImagePoint& point) {
	Astro::ImagePoint	result;
	result.x = point.x();
	result.y = point.y();
	return result;
}

// Image size

astro::image::ImageSize	convert(const Astro::ImageSize& size) {
	return ImageSize(size.width, size.height);
}

Astro::ImageSize	convert(const astro::image::ImageSize& size) {
	Astro::ImageSize	result;
	result.width = size.width();
	result.height = size.height();
	return result;
}

// Image rectangle

astro::image::ImageRectangle	convert(const Astro::ImageRectangle& rectangle) {
	return astro::image::ImageRectangle(convert(rectangle.origin),
						convert(rectangle.size));
}

Astro::ImageRectangle	convert(const astro::image::ImageRectangle& rectangle) {
	Astro::ImageRectangle	result;
	result.origin = convert(rectangle.origin());
	result.size = convert(rectangle.size());
	return result;
}

// shutter state

astro::camera::shutter_state    convert(const Astro::ShutterState& state) {
	switch (state) {
	case Astro::SHUTTER_OPEN:
		return astro::camera::SHUTTER_OPEN;
	case Astro::SHUTTER_CLOSED:
		return astro::camera::SHUTTER_CLOSED;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", state);
	throw std::runtime_error("illegal type");
}


Astro::ShutterState     convert(const astro::camera::shutter_state state) {
	switch (state) {
	case astro::camera::SHUTTER_OPEN:
		return Astro::SHUTTER_OPEN;
	case astro::camera::SHUTTER_CLOSED:
		return Astro::SHUTTER_CLOSED;
	}
	debug(LOG_ERR, DEBUG_LOG, 0, "illegal type: %d", state);
	throw std::runtime_error("illegal type");
}

// Binning mode
astro::camera::Binning	convert(const Astro::BinningMode& mode) {
	return astro::camera::Binning(mode.x, mode.y);
}

Astro::BinningMode	convert(const astro::camera::Binning& mode) {
	Astro::BinningMode	result;
	result.x = mode.getX();
	result.y = mode.getY();
	return result;
}

// Binning Set
astro::camera::BinningSet	convert(Astro::BinningSet_var set) {
	astro::camera::BinningSet	result;
	for (unsigned int i = 0; i < set->length(); i++) {
		result.insert(convert(set[i]));
	}
	return result;
}

astro::camera::BinningSet	convert(Astro::BinningSet set) {
	astro::camera::BinningSet	result;
	for (unsigned int i = 0; i < set.length(); i++) {
		result.insert(convert(set[i]));
	}
	return result;
}

Astro::BinningSet_var	convert(const astro::camera::BinningSet& set) {
	Astro::BinningSet	*resultset = new Astro::BinningSet();
	Astro::BinningSet_var	result = resultset;
	resultset->length(set.size());
	astro::camera::BinningSet::const_iterator	i;
	unsigned int	j = 0;
	for (i = set.begin(); i != set.end(); i++, j++) {
		result[j] = convert(*i);
	}
	return result;
}

// Exposure

astro::camera::Exposure	convert(const Astro::Exposure& exposure) {
	astro::camera::Exposure	result(convert(exposure.frame),
					exposure.exposuretime);
	result.gain = exposure.gain;
	result.limit = exposure.limit;
	if (exposure.limit > 0) {
		result.limit = exposure.limit;
	}
	result.shutter = convert(exposure.shutter);
	return result;
}

Astro::Exposure	convert(const astro::camera::Exposure& exposure) {
	Astro::Exposure	result;
	result.frame = convert(exposure.frame);
	result.exposuretime = exposure.exposuretime;
	result.gain = exposure.gain;
	result.limit = exposure.limit;
	result.shutter = convert(exposure.shutter);
	return result;
}

// relaybits
uint8_t	convert_octet2relaybits(CORBA::Octet bits) {
	uint8_t	result = 0;
	if (bits & Astro::GuiderPort::DECMINUS) {
		result |= astro::camera::GuiderPort::DECMINUS;
	}
	if (bits & Astro::GuiderPort::DECPLUS) {
		result |= astro::camera::GuiderPort::DECPLUS;
	}
	if (bits & Astro::GuiderPort::RAMINUS) {
		result |= astro::camera::GuiderPort::RAMINUS;
	}
	if (bits & Astro::GuiderPort::RAPLUS) {
		result |= astro::camera::GuiderPort::RAPLUS;
	}
	return result;
}

CORBA::Octet	convert_relaybits2octet(uint8_t bits) {
	CORBA::Octet	result = 0;
	if (bits & astro::camera::GuiderPort::DECMINUS) {
		result |= Astro::GuiderPort::DECMINUS;
	}
	if (bits & astro::camera::GuiderPort::DECPLUS) {
		result |= Astro::GuiderPort::DECPLUS;
	}
	if (bits & astro::camera::GuiderPort::RAMINUS) {
		result |= Astro::GuiderPort::RAMINUS;
	}
	if (bits & astro::camera::GuiderPort::RAPLUS) {
		result |= Astro::GuiderPort::RAPLUS;
	}
	return result;
}

// CcdInfo
Astro::CcdInfo	convert(const astro::camera::CcdInfo& info) {
	Astro::CcdInfo	result;
	result.name = CORBA::string_dup(info.name().c_str());
	result.id = info.getId();
	result.size = convert(info.size());
	result.binningmodes = convert(info.modes());
	result.shutter = info.shutter();
	result.pixelwidth = info.pixelwidth();
	result.pixelheight = info.pixelheight();
	return result;
}

astro::camera::CcdInfo	convert(const Astro::CcdInfo& info) {
	astro::camera::CcdInfo	result(std::string(info.name),
					convert(info.size), info.id);
	result.addModes(convert(info.binningmodes));
	result.setShutter(info.shutter);
	result.pixelwidth(info.pixelwidth);
	result.pixelheight(info.pixelheight);
	return result;
}

astro::camera::CcdInfo	convert(const Astro::CcdInfo_var& info) {
	astro::camera::CcdInfo	result(std::string(info->name),
					convert(info->size), info->id);
	result.addModes(convert(info->binningmodes));
	result.setShutter(info->shutter);
	result.pixelwidth(info->pixelwidth);
	result.pixelheight(info->pixelheight);
	return result;
}

} // namespace astro
