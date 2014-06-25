/*
 * ImagesI.h -- images device server
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImagesI_h
#define _ImagesI_h

#include <image.h>
#include <ImageDirectory.h>

namespace snowstar {

ImagePrx	getImage(const std::string& name, int bytesPerPixel,
			const Ice::Current& current);
ImagePrx	getImage(const std::string& name,
			astro::image::ImageDirectory& imagedirectory,
			const Ice::Current& current);

class ImagesI : public Images {
	astro::image::ImageDirectory&	imagedirectory;
public:
	ImagesI(astro::image::ImageDirectory& imagedirectory);
	virtual ~ImagesI();
	ImageList	listImages(const Ice::Current& current);
	int	imageSize(const std::string& name,
				const Ice::Current& current);
	int	imageAge(const std::string& name,
				const Ice::Current& current);
	ImagePrx	getImage(const std::string& name,
				const Ice::Current& current) {
		return snowstar::getImage(name, imagedirectory, current);
	}
};

} // namespace snowtar

#endif /* _ImagesI_h */
