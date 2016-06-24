/*
 * ImageQueueEntry.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

ImageQueueEntry::ImageQueueEntry(const Exposure& _exposure)
	: exposure(_exposure) {
}

ImageQueueEntry::ImageQueueEntry(const ImageQueueEntry& other)
	: exposure(other.exposure), image(other.image),
	  sequence(other.sequence) {
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
