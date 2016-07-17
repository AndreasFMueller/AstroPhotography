/*
 * ImageQueueEntry.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

ImageQueueEntry::ImageQueueEntry(const Exposure& _exposure)
	: exposure(_exposure), sequence(0) {
}

ImageQueueEntry::ImageQueueEntry(const Exposure& _exposure, ImagePtr _image)
	: exposure(_exposure), sequence(0), image(_image) {
}

ImageQueueEntry::ImageQueueEntry(const ImageQueueEntry& other)
	: exposure(other.exposure), sequence(other.sequence),
	  image(other.image) {
}

ImageQueueEntry&	ImageQueueEntry::operator=(
				const ImageQueueEntry& other) {
	exposure = other.exposure;
	image = other.image;
	sequence = other.sequence;
	return *this;
}

} // namespace camera
} // namespace astro
