/*
 * RefreshingTracker.cpp -- trackers that need to be refreshed from time to time
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <AstroFormat.h>
#include <AstroTransform.h>

using namespace astro::image::transform;
using namespace astro::adapter;

namespace astro {
namespace guiding {

/**
 * \brief Construct a refreshing tracker
 */
RefreshingTracker::RefreshingTracker() {
	_refreshinterval = 0;
	_lastimagetime = 0;
	_image = NULL;
}

/**
 * \brief Destroy the refreshing tracker
 */
RefreshingTracker::~RefreshingTracker() {
	// the image is just a copy of the pointer encapsulated by _imageptr,
	// so it is freed when _imageptr is destroyed. No need to do anything
	// in this destructor
}

/**
 * \brief Find out whether the image should be refreshed
 */
bool	RefreshingTracker::refreshNeeded() {
	// if the _refreshinterval is 0, then we never want to refresh
	if (_refreshinterval == 0) {
		return false;
	}
	// if the _refreshinterval has passed since the last time an
	// image was taken, then wee need to refresh
	return (Timer::gettime() > _lastimagetime + _refreshinterval);
}

/**
 * \brief String representation of the tracker
 */
std::string	RefreshingTracker::toString() const {
	return stringprintf("%s %s refresh=%d offset=%s",
		demangle(typeid(*this).name()).c_str(),
		(_image) ? _image->size().toString().c_str() : "(no image)",
		_refreshinterval, _offset.toString().c_str());
}

/**
 * \brief Refresh by creating a copy of the image, and updating the offset
 */
void	RefreshingTracker::refresh(const ConstImageAdapter<double>& adapter,
		const Point offset) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "refreshing with image %s",
		_image->size().toString().c_str());
	_image = new Image<double>(adapter);
	_imageptr = ImagePtr(_image);
	_offset = _offset + offset;
	_lastimagetime = Timer::gettime();
}

/**
 * \brief Perform phase correlation
 *
 * This method performs the phase correlation and constructs a new image
 * if a refresh is needed.
 */
Point	RefreshingTracker::correlate(const ConstImageAdapter<double>& adapter,
		PhaseCorrelator& correlator) {
	Point   offset = correlator(*_image, adapter).first;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "correlate %s with %s -> %s",
		_image->size().toString().c_str(),
		adapter.getSize().toString().c_str(),
		offset.toString().c_str());
	if (refreshNeeded()) {
		refresh(adapter, offset);
	}
	return _offset + offset;
}

Point	RefreshingTracker::correlate(const ConstImageAdapter<double>& adapter) {
	PhaseCorrelator	pc;
	return correlate(adapter, pc);
}

#define	construct(Pixel, newimage)					\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != imagep) {						\
		return new LuminanceAdapter<Pixel, double>(*imagep);	\
	}								\
}

/**
 * \brief Construct a luminance adapter for the image
 *
 * All the phase correlation trackers operate on the luminance channel only,
 * so we provide this method in the base class to extract the luminance in
 * double format independently of the pixel type provided by the camera.
 */
ConstImageAdapter<double>	*RefreshingTracker::adapter(const ImagePtr image) {
	construct(unsigned char, image);
	construct(unsigned short, image);
	construct(unsigned int, image);
	construct(unsigned long, image);
	construct(float, image);
	construct(double, image);
	construct(RGB<unsigned char>, image);
	construct(RGB<unsigned short>, image);
	construct(RGB<unsigned int>, image);
	construct(RGB<unsigned long>, image);
	construct(RGB<float>, image);
	construct(RGB<double>, image);
	construct(YUYV<unsigned char>, image);
	construct(YUYV<unsigned short>, image);
	construct(YUYV<unsigned int>, image);
	construct(YUYV<unsigned long>, image);
	construct(YUYV<float>, image);
	construct(YUYV<double>, image);
	throw std::runtime_error("cannot track this image type");
}

} // namespace guiding
} // namespace astro
