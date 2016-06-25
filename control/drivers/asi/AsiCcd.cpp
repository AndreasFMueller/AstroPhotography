/*
 * AsiCcd.cpp -- implementation of asi ccd
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rappreswil
 */

#include <AsiCcd.h>
#include <ASICamera2.h>

namespace astro {
namespace camera {
namespace asi {

AsiCcd::AsiCcd(const CcdInfo& info, AsiCamera& camera)
	: Ccd(info), _camera(camera) {
}

AsiCcd::~AsiCcd() {
}

std::string	AsiCcd::imgtype2string(int imgtype) {
	switch (imgtype) {
	case ASI_IMG_RAW8:
		return std::string("raw8");
	case ASI_IMG_RGB24:
		return std::string("rgb24");
	case ASI_IMG_RAW16:
		return std::string("raw16");
	case ASI_IMG_Y8:
		return std::string("y8");
	}
	throw std::runtime_error("unknown image type");
}

int	AsiCcd::string2imgtype(const std::string& imgname) {
	if (imgname == "raw8") {
		return ASI_IMG_RAW8;
	}
	if (imgname == "rgb24") {
		return ASI_IMG_RGB24;
	}
	if (imgname == "raw16") {
		return ASI_IMG_RAW16;
	}
	if (imgname == "y8") {
		return ASI_IMG_Y8;
	}
	throw std::runtime_error("unknown image name");
}

void	AsiCcd::startExposure(const Exposure& /* exposure */) {
}

CcdState::State	AsiCcd::exposureStatus() {
	ASI_EXPOSURE_STATUS	status;
	if (ASI_SUCCESS != ASIGetExpStatus(_camera.index(), &status)) {
		std::string	msg = stringprintf("cannot get exp status @ %d",
			_camera.index());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	switch (status) {
	case ASI_EXP_IDLE:
		return CcdState::idle;
	case ASI_EXP_WORKING:
		return CcdState::exposing;
	case ASI_EXP_SUCCESS:
		return CcdState::exposed;
	case ASI_EXP_FAILED:
		return CcdState::exposed;
	}
}

astro::image::ImagePtr	AsiCcd::getRawImage() {
	return astro::image::ImagePtr(NULL);
}

CoolerPtr	AsiCcd::getCooler0() {
	return CoolerPtr(NULL);
}

} // namespace asi
} // namespace camera
} // namespace astro
