/*
 * ImageQueue.cpp -- a queue of images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>

namespace astro {
namespace camera {

ImageQueue::ImageQueue(unsigned long maxqueuelength)
	: _maxqueuelength(maxqueuelength) {
	_processed = 0;
	_dropped = 0;
	_sequence = 0;
}

bool	ImageQueue::hasImage() {
	std::unique_lock<std::mutex>	lock(mutex);
	return (queue.size() > 0) ? true : false;
}

ImageQueueEntry	ImageQueue::get(bool block) throw (EmptyQueue) {
	std::unique_lock<std::mutex>	lock(mutex);
	while (true) {
		if (queue.size() > 0) {
			ImageQueueEntry	result = queue.front();
			queue.pop_front();
			return result;
		}
		if (!block) {
			throw EmptyQueue();
		}
		condition.wait(lock);
	}
}

ImagePtr	ImageQueue::getImage(bool block) {
	return get(block).image;
}

void	ImageQueue::add(const Exposure& exposure, ImagePtr image) {
	std::unique_lock<std::mutex>	lock(mutex);
	if (queue.size() < _maxqueuelength) {
		ImageQueueEntry	entry(exposure);
		entry.sequence = _sequence++;
		entry.image = image;
		queue.push_back(entry);
	} else {
		_dropped++;
	}
	_processed++;
	condition.notify_all();
}

} // namespace camera
} // namespace astro
