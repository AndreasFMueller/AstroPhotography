/*
 * GuiderImages.cpp -- Implementation of calibration image related methods
 *                     of the Guider class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>

#include <AstroGuiding.h>
#include <AstroCallback.h>
#include <AstroUtils.h>
#include <AstroAdapter.h>

using namespace astro::image;
using namespace astro::camera;
using namespace astro::callback;

namespace astro {
namespace guiding {

/**
 * \brief Callback class to signal the end of the guide process
 */
class DarkCallback : public Callback {
	Guider&	_guider;
public:
	DarkCallback(Guider& guider) : _guider(guider) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "DarkCallback created");
	}
	CallbackDataPtr	operator()(CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "DarkCallback callback called");
		CalibrationImageProgressData	*d
			= dynamic_cast<CalibrationImageProgressData*>(&*data);
		if (d) {
			_guider.GuiderBase::callback(d->data());
			if (d->data().imageno < 0) {
				_guider.endDark();
			}
		}
		return data;
	}
};

/**
 * \brief Start getting a dark image
 *
 * \param exposuretime	exposure time to use for darks
 * \param imagecount	number of images to use to construct the dark
 */
void	Guider::startDark(double exposuretime, int imagecount,
		double badpixellimit) {
	_state.startDarkAcquire();
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start to acquire a dark");
		// set up the collback
		DarkCallback	*darkcallback = new DarkCallback(*this);
		CallbackPtr	darkcallbackptr(darkcallback);

		// set up the dark work
		_darkwork = DarkWorkImagerPtr(new DarkWorkImager(imager()));
		_darkwork->exposuretime(exposuretime);
		_darkwork->imagecount(imagecount);
		_darkwork->badpixellimit(badpixellimit);
		_darkwork->callback(darkcallbackptr);

		// set iup the thread
		DarkWorkImagerThread	*dwit
			= new DarkWorkImagerThread(&*_darkwork);
		_darkthread = DarkWorkImagerThreadPtr(dwit);

		// start the thread
		_darkthread->start();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dark acquire is running");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"dark acquisition start failed: %s", x.what());
		callback(x);
	}
}

/**
 * \brief Method to signal the end of the dark acquisition process
 */
void	Guider::endDark() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dark ended");
	_state.endDarkAcquire();
}

/**
 * \brief Callback class to signal the end of the guide process
 */
class FlatCallback : public Callback {
	Guider&	_guider;
public:
	FlatCallback(Guider& guider) : _guider(guider) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "create FlatCallback");
	}
	CallbackDataPtr	operator()(CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "FlatCallback called");
		if (!data) {
			return data;
		}
		CalibrationImageProgressData	*d
			= dynamic_cast<CalibrationImageProgressData*>(&*data);
		if (d) {
			_guider.GuiderBase::callback(d->data());
			if (d->data().imageno < 0) {
				_guider.endFlat();
			}
		}
		return data;
	}
};

/**
 * \brief Start getting a flat image
 *
 * \param exposuretime	exposure time to use for flats
 * \param imagecount	number of images to use to construct the flat
 */
void	Guider::startFlat(double exposuretime, int imagecount, bool useDark) {
	_state.startFlatAcquire();
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start to acquire a flat");
		// preparing the flat callback
		FlatCallback	*flatcallback = new FlatCallback(*this);
		CallbackPtr	flatcallbackptr(flatcallback);

		// set up the flat work
		_flatwork = FlatWorkImagerPtr(new FlatWorkImager(imager()));
		_flatwork->exposuretime(exposuretime);
		_flatwork->imagecount(imagecount);
		if (useDark) {
			_flatwork->darkimage(imager().dark());
		}
		_flatwork->callback(flatcallbackptr);

		// set iup the thread
		FlatWorkImagerThread	*dwit
			= new FlatWorkImagerThread(&*_flatwork);
		_flatthread = FlatWorkImagerThreadPtr(dwit);

		// start the thread
		_flatthread->start();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "flat acquire is running");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0,
			"flat acquisition start failed: %s", x.what());
		callback(x);
	}
}

/**
 * \brief Method to signal the end of the flat acquisition process
 */
void	Guider::endFlat() {
	_state.endFlatAcquire();
}

/**
 * \brief Callback class to signal the end of the guide process
 */
class ImageEndCallback : public Callback {
	Guider&	_guider;
public:
	ImageEndCallback(Guider& guider) : _guider(guider) { }
	CallbackDataPtr	operator()(CallbackDataPtr data) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "endImage callback called");
		if (!data) {
			_guider.endImaging(ImagePtr(NULL));
		} else {
			ImageCallbackData	*icd
				= dynamic_cast<ImageCallbackData*>(&*data);
			if (icd) {
				_guider.endImaging(icd->image());
			} else {
				_guider.endImaging(ImagePtr(NULL));
			}
		}
		return data;
	}
};

/**
 * \brief Start acquiring an image via the imager
 *
 * \param exposure	exposure settings to use
 */
void	Guider::startImaging(const Exposure& exposure) {
	_state.startImaging();
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "start to acquire image %s",
			exposure.toString().c_str());
		_imagework = ImageWorkImagerPtr(
				new ImageWorkImager(imager(), exposure));
		_imagework->endcallback(CallbackPtr(new ImageEndCallback(*this)));
		// set iup the thread
		ImageWorkImagerThread	*iwit
			= new ImageWorkImagerThread(&*_imagework);
		_imagethread = ImageWorkImagerThreadPtr(iwit);

		// start the thread
		_imagethread->start();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "imaging process is running");
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "image acquisition failed: %s",
			x.what());
		callback(x);
	}
}

/**
 * \brief Method to signal the end of the image acquisition process
 */
void	Guider::endImaging(ImagePtr image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "received an image");
	_imaging = image;
	if (_imaging) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "Image size: %s",
			_imaging->size().toString().c_str());
	}
	_state.endImaging();
}

} // namespace guiding
} // namespace astro
