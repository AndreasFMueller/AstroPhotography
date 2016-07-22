/*
 * ImageSinkI.h -- implementation of a generic image sink
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageSinkI_h
#define _ImageSinkI_h

#include <camera.h>
#include <Ice/Ice.h>

namespace snowstar {

/**
 * \brief Base class of all image sink implementations
 */
class ImageSinkI : public ImageSink {
public:
	ImageSinkI();
	void	stop(const Ice::Current& current);
	void	image(const ImageQueueEntry& entry,
			const Ice::Current& current);
};

} // namespace snowstar

#endif /* _ImageSink_h */
