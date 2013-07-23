/*
 * Guiding.cpp -- classes implementing guiding
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>

using namespace astro::image;
using namespace astro::image::transform;

namespace astro {
namespace guiding {

#define	findstar_typed(Pixel)						\
{									\
	Image<Pixel >	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*image);		\
	if (NULL != imagep) {						\
		StarDetector<Pixel >	sd(*imagep);			\
		return sd(rectangle, k);				\
	}								\
}

Point	findstar(ImagePtr image, const ImageRectangle& rectangle,
		unsigned int k) {
	findstar_typed(unsigned char);
	findstar_typed(unsigned short);
	findstar_typed(unsigned int);
	findstar_typed(unsigned long);
	findstar_typed(float);
	findstar_typed(double);
	findstar_typed(RGB<unsigned char>);
	findstar_typed(RGB<unsigned short>);
	findstar_typed(RGB<unsigned int>);
	findstar_typed(RGB<unsigned long>);
	findstar_typed(RGB<float>);
	findstar_typed(RGB<double>);
	findstar_typed(YUYV<unsigned char>);
	findstar_typed(YUYV<unsigned short>);
	findstar_typed(YUYV<unsigned int>);
	findstar_typed(YUYV<unsigned long>);
	findstar_typed(YUYV<float>);
	findstar_typed(YUYV<double>);
	throw std::runtime_error("cannot find star in this image type");
}

StarTracker::StarTracker(const astro::image::transform::Point& _point,
	const ImageRectangle& _rectangle, unsigned int _k)
	: point(_point), rectangle(_rectangle), k(_k) {
}

astro::image::transform::Point	StarTracker::operator()(ImagePtr newimage)
	const {
	// find the star on the new image
	Point	newpoint = findstar(newimage, rectangle, k);
	return newpoint - point;
}

#define	phasetracker_construct(Pixel)					\
{									\
	Image<Pixel>	*imagep						\
		= dynamic_cast<Image<Pixel > *>(&*_image);		\
	if (NULL != imagep) {						\
		LuminanceAdapter<Pixel >	la(*imagep);		\
		image = ImagePtr(new Image<double>(la));		\
		return;							\
	}								\
}

PhaseTracker::PhaseTracker(ImagePtr _image) {
	phasetracker_construct(unsigned char);
	phasetracker_construct(unsigned short);
	phasetracker_construct(unsigned int);
	phasetracker_construct(unsigned long);
	phasetracker_construct(float);
	phasetracker_construct(double);
	phasetracker_construct(RGB<unsigned char>);
	phasetracker_construct(RGB<unsigned short>);
	phasetracker_construct(RGB<unsigned int>);
	phasetracker_construct(RGB<unsigned long>);
	phasetracker_construct(RGB<float>);
	phasetracker_construct(RGB<double>);
	phasetracker_construct(YUYV<unsigned char>);
	phasetracker_construct(YUYV<unsigned short>);
	phasetracker_construct(YUYV<unsigned int>);
	phasetracker_construct(YUYV<unsigned long>);
	phasetracker_construct(YUYV<float>);
	phasetracker_construct(YUYV<double>);
	throw std::runtime_error("cannot track this image type");
}

#define	phasetracker_typed(Pixel)					\
{									\
	Image<Pixel>	*newimagep					\
		= dynamic_cast<Image<Pixel > *>(&*newimage);		\
	if (NULL != newimagep) {					\
		LuminanceAdapter<Pixel >	newla(*newimagep);	\
		Image<double>	*imagep					\
			= dynamic_cast<Image<double> *>(&*image);	\
		PhaseCorrelator	pc;					\
		return pc(*imagep, newla);				\
	}								\
}

astro::image::transform::Point	PhaseTracker::operator()(ImagePtr newimage)
	const {
	phasetracker_typed(unsigned char);
	phasetracker_typed(unsigned short);
	phasetracker_typed(unsigned int);
	phasetracker_typed(unsigned long);
	phasetracker_typed(float);
	phasetracker_typed(double);
	phasetracker_typed(RGB<unsigned char>);
	phasetracker_typed(RGB<unsigned short>);
	phasetracker_typed(RGB<unsigned int>);
	phasetracker_typed(RGB<unsigned long>);
	phasetracker_typed(RGB<float>);
	phasetracker_typed(RGB<double>);
	phasetracker_typed(YUYV<unsigned char>);
	phasetracker_typed(YUYV<unsigned short>);
	phasetracker_typed(YUYV<unsigned int>);
	phasetracker_typed(YUYV<unsigned long>);
	phasetracker_typed(YUYV<float>);
	phasetracker_typed(YUYV<double>);
	throw std::runtime_error("cannot track this image type");
}

} // namespace guiding
} // namespace astro
