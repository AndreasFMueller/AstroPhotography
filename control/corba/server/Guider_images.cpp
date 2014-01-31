/*
 * Guider_images.cpp -- implementation of methods related to images produced
 *                      during calibration or tracking
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Guider_impl.h>
#include <Camera_impl.h>
#include <Ccd_impl.h>
#include <GuiderPort_impl.h>
#include <ServantBuilder.h>
#include <AstroCallback.h>
#include <AstroUtils.h>
#include <AstroFormat.h>
#include <ImageObjectDirectory.h>
#include <TrackingPersistence.h>
#include <Conversions.h>
#include <GuiderImageCallback.h>
#include <TrackingInfoCallback.h>
#include <CalibrationPointCallback.h>
#include <GuiderFactory_impl.h>

namespace Astro {

/**
 * \brief get most recent image
 */
Image_ptr	Guider_impl::mostRecentImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve most recent image");
	// actuall retrieve the most recent image from the callback
	ImagePtr	image = _guider->mostRecentImage;
	if (!image) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "there is no most recent image");
		throw CORBA::OBJECT_NOT_EXIST();
	}

	// save the image in the image directory
        Astro::ImageObjectDirectory    directory;
        std::string     filename = directory.save(image);

        // activate this object
        return directory.getImage(filename);
}

/**
 * \brief Register a tracking image monitor
 */
::CORBA::Long	Guider_impl::registerImageMonitor(::Astro::TrackingImageMonitor_ptr monitor) {
	return trackingimagechannel.subscribe(monitor);
}

/**
 * \brief Unregister a imagemonitor id
 *
 * \param monitorid	This is the image monitor id returned by the
 *			register call
 */
void	Guider_impl::unregisterImageMonitor(::CORBA::Long imagemonitorid) {
	trackingimagechannel.unsubscribe(imagemonitorid);
}

/**
 * \brief update distribution function
 */
void	Guider_impl::update(const ::Astro::TrackingImage& image) {
	trackingimagechannel.update(image);
}

/**
 * \brief Inform the clients that guiding has stopped
 */
void	Guider_impl::trackingimage_stop() {
	trackingimagechannel.stop();
}

} // namespace Astro
