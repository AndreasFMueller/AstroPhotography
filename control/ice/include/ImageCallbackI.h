/*
 * ImageCallbackI.h -- implementation of a generic image callback
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
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
	void	update(const SimpleImage& image, const Ice::Current& current);
};

} // namespace snowstar
