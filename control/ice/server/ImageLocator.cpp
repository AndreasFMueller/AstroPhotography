/*
 * ImageLocator.cpp -- ImageLocator implementation
 *
 * (c) 2014 Prof DR Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageLocator.h>
#include <ImageI.h>
#include <AstroDebug.h>
#include <AstroFilterfunc.h>
#include <ImageDirectory.h>

namespace snowstar {

// never maintain more than maxservants
static const int	maxservants = 120;

// method to launch the image expiration thread
static void	launch_expiration(ImageLocator *imagelocator) {
	imagelocator->run();
}

/**
 * \brief Constructor for an ImageLocator
 */
ImageLocator::ImageLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image locator created");
	_stop = false;
	_thread = std::thread(launch_expiration, this);
}

/**
 * \brief Destructor for the ImageLocator
 */
ImageLocator::~ImageLocator() {
	stop();
	if (_thread.joinable()) {
		_thread.join();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroy the image locator");
}

/**
 * \brief locate an image
 *
 * This method creates an image servant of the correct pixel type
 */
Ice::ObjectPtr	ImageLocator::locate(const Ice::Current& current,
					Ice::LocalObjectPtr& /* cookie */) {
	std::unique_lock<std::mutex>	lock(_mutex);

	std::string	name = current.id.name;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get image %s", name.c_str());

	// see whether we can satisfy the request from the cache
	imagemap::iterator	i = _images.find(name);
	if (i != _images.end()) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s found in cache",
			name.c_str());
		return i->second;
	}

	// have to create a new servant
	astro::image::ImageDirectory	_imagedirectory;
	Ice::ObjectPtr	ptr;
	if (!_imagedirectory.isFile(name)) {
		std::string	msg = astro::stringprintf(
			"image file %s not found", name.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw NotFound(msg);
	}
	astro::image::ImagePtr	image = _imagedirectory.getImagePtr(name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %s image with %s pixels",
		image->size().toString().c_str(),
		astro::demangle(image->pixel_type().name()).c_str());

	// find build the image proxy matching the pixel type
	std::type_index	type = image->pixel_type();
	if ((type == typeid(unsigned char))
		|| (type == typeid(astro::image::YUYV<unsigned char>))
		|| (type == typeid(astro::image::RGB<unsigned char>))) {
		ptr = new ByteImageI(image, name);
	}
	if ((type == typeid(unsigned short))
		|| (type == typeid(astro::image::YUYV<unsigned short>))
		|| (type == typeid(astro::image::RGB<unsigned short>))) {
		ptr = new ShortImageI(image, name);
	}
	if ((type == typeid(unsigned int))
		|| (type == typeid(astro::image::YUYV<unsigned int>))
		|| (type == typeid(astro::image::RGB<unsigned int>))) {
		ptr = new IntImageI(image, name);
	}
	if ((type == typeid(float))
		|| (type == typeid(astro::image::YUYV<float>))
		|| (type == typeid(astro::image::RGB<float>))) {
		ptr = new FloatImageI(image, name);
	}
	if ((type == typeid(double))
		|| (type == typeid(astro::image::YUYV<double>))
		|| (type == typeid(astro::image::RGB<double>))) {
		ptr = new DoubleImageI(image, name);
	}
	if (!ptr) {
		BadParameter	exception;
		std::string	msg = astro::stringprintf("don't know how to "
			"handle %s pixels",
			astro::demangle(type.name()).c_str());
		exception.cause = msg;
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw exception;
	}

	// look for the oldest image and remove it from the map until 
	while (_images.size() > maxservants) {
		removeoldest();
	}

	// add the servant to the cache
	_images.insert(std::make_pair(name, ptr));
	return ptr;
}

/**
 * \brief finished locator interface method
 */
void	ImageLocator::finished(const Ice::Current& /* current */,
				const Ice::ObjectPtr& /* servant */,
				const Ice::LocalObjectPtr& /* cookie */) {
}

/**
 * \brief deactivate locator interface method
 */
void	ImageLocator::deactivate(const std::string& category) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "deactivate: %s", category.c_str());
}

/**
 * \brief Expire all images
 *
 * This method calls the expire method on all images and counts how many
 * times it actually did expire an image. This allows to get some information
 * about how reliably clients clean up the images.
 */
void	ImageLocator::expire() {
	imagemap::iterator	i;
	int	counter = 0;
	for (i = _images.begin(); i != _images.end(); i++) {
		ImageI	*im = dynamic_cast<ImageI*>(&*i->second);
		if (im) {
			if (im->expire()) {
				debug(LOG_DEBUG, DEBUG_LOG, 0,
					"image '%s' expired",
					im->filename().c_str());
				counter++;
			}
		}
	}
	if (counter > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d images expired", counter);
	}
}

/**
 * \brief Work method for the expiration thread
 *
 * The trampoline function launch_expiration just calls this method
 */
void	ImageLocator::run() {
	std::unique_lock<std::mutex>	lock(_mutex);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start image expiration thread");
	do {
		expire();
		_condition.wait_for(lock, std::chrono::seconds(10));
	} while (!_stop);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image expiration thread completes");
}

/**
 * \brief stop the expiration thread
 *
 * This method sets the _stop signal to true and signals the thread that
 * it should stop expiring.
 */
void	ImageLocator::stop() {
	std::unique_lock<std::mutex>	lock(_mutex);
	_stop = true;
	_condition.notify_all();
}

/**
 * \brief remove the oldest image from the map
 */
void	ImageLocator::removeoldest() {
	// don't do anything 
	if (_images.size() == 0) {
		return;
	}
	imagemap::const_iterator	oldest = _images.begin();
	ImageI	*oldestim = dynamic_cast<ImageI*>(&*oldest->second);
	if (NULL == oldestim) {
		debug(LOG_ERR, DEBUG_LOG, 0, "not an image (internal error)");
		return;
	}
	time_t	oldesttime = oldestim->lastused();

	// now go through all the images and found an older one
	imagemap::const_iterator	i;
	for (i = _images.begin(); i != _images.end(); i++) {
		ImageI	*im = dynamic_cast<ImageI*>(&*i->second);
		if (im) {
			time_t	t = im->lastused();
			if (t < oldesttime) {
				oldest = i;
				oldesttime = t;
			}
		}
	}

	// we have an image
	if (oldest != _images.end()) {
		time_t	now;
		time(&now);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"remove servant for '%s', age %d",
			oldest->first.c_str(), now - oldesttime);
		_images.erase(oldest);
	}
}

} // namespace snowstar
