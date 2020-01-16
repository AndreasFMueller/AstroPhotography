/*
 * ImageCallbackI.h -- implementation of a generic image callback
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageCallbackI_h
#define _ImageCallbackI_h

#include <image.h>
#include <Ice/Ice.h>

namespace snowstar {

/**
 * \brief A generic callback to process images received from the snowstar server
 */
class ImageCallbackI : public ImageMonitor {
	std::string	_path;
	std::string	_prefix;
	int	imagecount;
public:
	ImageCallbackI(const std::string& path, const std::string& prefix);
	void	stop(const Ice::Current& current);
	void	update(const ImageBuffer& image, const Ice::Current& current);
};

} // namespace snowstar

#endif /* _ImageCallback_h */
