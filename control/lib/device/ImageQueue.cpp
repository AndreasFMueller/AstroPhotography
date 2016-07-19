/*
 * ImageQueue.cpp -- a queue of images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCamera.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {

/**
 * \brief Constructor for an ImageQueue object
 *
 * The queue is configured to hold only a limited number of entries, 10
 * by default. If new 
 *
 * \param maxqueuelength	the maximum number of entries in the queue
 */
ImageQueue::ImageQueue(unsigned long maxqueuelength)
	: _maxqueuelength(maxqueuelength) {
	_processed = 0;
	_dropped = 0;
	_sequence = 0;
}

/**
 * \brief Check whether there are images in the queue
 */
bool	ImageQueue::hasEntry() {
	std::unique_lock<std::mutex>	lock(mutex);
	return (queue.size() > 0) ? true : false;
}

/**
 * \brief Retrieve an entry from the queue
 *
 * \param block		whether or not to wait for a new image to arrive
 *			in the queue
 */
ImageQueueEntry	ImageQueue::getEntry(bool block) throw (EmptyQueue) {
	std::unique_lock<std::mutex>	lock(mutex);
	while (true) {
		if (queue.size() > 0) {
			ImageQueueEntry	result = queue.front();
			queue.pop_front();
			return result;
		}
		if (!block) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "queue is empty");
			throw EmptyQueue();
		}
		condition.wait(lock);
	}
}

/**
 * \brief Add an image to the queue
 *
 * \param exposure	Exposure parameters used for this image
 * \param image		the image itself
 */
void	ImageQueue::add(const Exposure& exposure, ImagePtr image) {
	ImageQueueEntry	entry(exposure, image);
	add(entry);
}

/**
 * \brief Add an entry to the queue
 *
 * As a side effect, the entry contains the sequence number in the queue
 * after it was added to the queue
 *
 * \param entry		queue entry to add
 */
void	ImageQueue::add(ImageQueueEntry& entry) {
	std::unique_lock<std::mutex>	lock(mutex);
	_processed++;
	if (queue.size() < _maxqueuelength) {
		entry.sequence = _sequence++;
		queue.push_back(entry);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add image, queue length now %d",
			queue.size());
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dropping image %s (%u/%d)",
			entry.image->size().toString().c_str(),
			queue.size(), _maxqueuelength);
		_dropped++;
		throw ImageDropped();
	}
	condition.notify_all();
}

} // namespace camera
} // namespace astro
