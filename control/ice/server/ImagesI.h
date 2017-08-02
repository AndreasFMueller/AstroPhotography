/*
 * ImagesI.h -- images device server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImagesI_h
#define _ImagesI_h

#include <image.h>
#include <typeindex>

namespace snowstar {

ImagePrx	getImage(const std::string& name, std::type_index type,
			const Ice::Current& current);
ImagePrx	getImage(const std::string& name,
			const Ice::Current& current);

class ImagesI : virtual public Images {
public:
	ImagesI();
	virtual ~ImagesI();
	ImageList	listImages(const Ice::Current& current);
	int	imageSize(const std::string& name,
				const Ice::Current& current);
	int	imageAge(const std::string& name,
				const Ice::Current& current);
	ImagePrx	getImage(const std::string& name,
				const Ice::Current& current) {
		return snowstar::getImage(name, current);
	}
};

} // namespace snowtar

#endif /* _ImagesI_h */
