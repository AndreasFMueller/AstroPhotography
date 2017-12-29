/*
 * FWHM.cpp -- tools for computing the FWHM 
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFWHM.h>
#include <AstroFilter.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroFormat.h>

namespace astro {
namespace image {
namespace fwhm {

std::string	ComponentInfo::toString() const {
	return stringprintf("%d: center=%s, radius=%f, rep=%s, size=%d",
		(int)_label, _center.toString().c_str(), _radius,
		_representant.toString().c_str(), _size);
}

// Plan:
// 1. For each pixel that attains the maximum value, find the connected
//    component of the pixels >= max/2
// 2. Compute the radius and the center of the circle enclosing the 
//    component
// 3. Find the maximum point closest to the center of the circle
// 4. Return a list of elements that contain center, radius, representative
//    maximum point and optionally an image illustrating all this information

template<typename Pixel>
void	componentAnalysisInitialize(Image<unsigned char>& components,
		const Image<Pixel>& image, Pixel limit = 0) {
	int	w = image.size().width();
	int	h = image.size().height();
	Pixel	maximum = std::numeric_limits<Pixel>::max();
	if (maximum / 2 > limit) {
		maximum = 2 * limit;
	}

	// 1. find the maximum value
	if (limit == 0) {
		filter::Max<Pixel, Pixel>       m;
		maximum = m(image);
		limit = maximum / 2;
	}

	// statistics
	int	maxima = 0;
	int	bright = 0;

	// 2. mark all pixels according to whether they are = maximum
	//    or >= maximum/2
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			Pixel	v = image.pixel(x, y);
			if (v == maximum) {
				components.pixel(x, y) = 255;
				maxima++;
			} else if (v > limit) {
				components.pixel(x, y) = 1;
				bright++;
			} else {
				components.pixel(x, y) = 0;
			}
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d maxima and %d bright points",
		maxima, bright);
}

/**
 * \brief Growing a component
 *
 */
size_t	growComponent(Image<unsigned char>& component, unsigned char label) {
	int	w = component.size().width() - 1;
	int	h = component.size().height() - 1;
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "growing component %d (%dx%d)",
	//	(int)label, w + 1, h + 1);
	size_t	counter = 0;
	for (int x = 0; x <= w; x++) {
		for (int y = 0; y <= h; y++) {
			unsigned char	v = component.pixel(x, y);
			// we can only change pixels that have value 1 or 255
			if ((v != 1) && (v != 255))  {
				continue;
			}
			if ((x > 0) && (component.pixel(x - 1, y) == label)) {
				component.pixel(x, y) = label;
				counter++;
				continue;
			}
			if ((y > 0) && (component.pixel(x, y - 1) == label)) {
				component.pixel(x, y) = label;
				counter++;
				continue;
			}
			if ((x < w) && (component.pixel(x + 1, y) == label)) {
				component.pixel(x, y) = label;
				counter++;
				continue;
			}
			if ((y < h) && (component.pixel(x, y + 1) == label)) {
				component.pixel(x, y) = label;
				counter++;
				continue;
			}
		}
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "component %d grown by %d pixels",
	//	(int)label, counter);
	return counter;
}


/**
 * \brief Build a component with a label starting at a point
 *
 * \param component	image that holds the component labels
 * \param image		image to analyze
 * \param start		starting point for creating 
 * \param label		number to mark points belonging to this component with
 */
size_t	componentAnalysisAt(Image<unsigned char>& component,
		const ImagePoint& start, unsigned char label) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "build component %d at %s",
		(int)label, start.toString().c_str());
	if (component.pixel(start.x(), start.y()) != 255) {
		throw std::logic_error("can only start growing a component "
			"at a maximum point");
	}
	component.pixel(start.x(), start.y()) = label;
	size_t	counter = 0;

	// now iterate growing the component until it no longer changes
	size_t	componentsize = 1;
	int	iterations = 0;
	while ((counter = growComponent(component, label)) > 0) {
		componentsize += counter;
		iterations++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "component %d required %d iterations",
		(int)label, iterations);
	return componentsize;
}

/**
 * \brief got through the image and partition into connectedcomponents
 */
template<typename Pixel>
int	componentAnalysis(Image<unsigned char>& component,
		const ConstImageAdapter<Pixel>& image, Pixel limit = 0) {
	// initialze the component analysis image
	componentAnalysisInitialize<Pixel>(component, image, limit);

	// get the image size parameters
	int	w = image.getSize().width();
	int	h = image.getSize().height();

	// perform component analysis for each point of the component image
	unsigned char	label = 2;
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			if (component.pixel(x, y) == 255) {
				componentAnalysisAt(component,
					ImagePoint(x, y), label);
				if (label == 254) {
					throw std::runtime_error("too many "
						"components");
				}
				label++;
			}
		}
	}

	// return the number of components found
	return label - 2;
}

#define analyzeComponents(Pixel)					\
{									\
	Image<Pixel>	*imgptr						\
		= dynamic_cast<Image<Pixel>*>(&*rawimage);		\
	if (NULL != imgptr) {						\
		componentcount = componentAnalysis<Pixel>(_image,	\
			 *imgptr, limit);				\
	}								\
}

#define analyzeComponentsRGB(Pixel)					\
{									\
	Image<RGB<Pixel> >	*imgptr					\
		= dynamic_cast<Image<RGB<Pixel> >*>(&*rawimage);	\
	if (NULL != imgptr) {						\
		adapter::ColorGreenAdapter<Pixel> greenadapter(*imgptr);\
		componentcount = componentAnalysis<Pixel>(_image,	\
			 greenadapter, limit);				\
	}								\
}



/**
 * \brief Construct a ComponentDecomposer
 */
ComponentDecomposer::ComponentDecomposer(ImagePtr rawimage, bool with_images, double limit)
	: _image(rawimage->size()), _with_images(with_images) {
	// anaylize components (this is type dependent)
	int	componentcount = 0;
	analyzeComponents(unsigned char);
	analyzeComponents(unsigned short);
	analyzeComponents(unsigned int);
	analyzeComponents(unsigned long);
	analyzeComponents(float);
	analyzeComponents(double);
	analyzeComponentsRGB(unsigned char);
	analyzeComponentsRGB(unsigned short);
	analyzeComponentsRGB(unsigned int);
	analyzeComponentsRGB(unsigned long);
	analyzeComponentsRGB(float);
	analyzeComponentsRGB(double);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d components", componentcount);
	if (componentcount == 0) {
		throw std::logic_error("no components found");
	}

	// find ComponentInfo for each component
	for (unsigned char label = 2; label < componentcount + 2; label++) {
		_components.push_back(component(label));
	}
}

/**
 * \brief Analyze a component
 */
ComponentInfo	ComponentDecomposer::component(unsigned char label) const {
	std::list<ImagePoint>	points;
	int	w = _image.size().width();
	int	h = _image.size().height();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			if (_image.pixel(x, y) == label) {
				points.push_back(ImagePoint(x, y));
			}
		}
	}
	ComponentInfo	info;
	info._label = label;
	info._radius = filter::MinRadius(points, info._center);
	info._size = 0;

	// create 
	Image<unsigned char>	*comp = NULL;
	if (_with_images) {
		comp = new Image<unsigned char>(_image.size());
		info._image = ImagePtr(comp);
	}

	// find the point closest to the center
	double	d = std::numeric_limits<double>::max();
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			if (_image.pixel(x, y) == label) {
				ImagePoint	p(x, y);
				double	l = (info._center - p).abs();
				if (l < d) {
					d = l;
					info._representant = p;
				}
				if (_with_images) {
					comp->pixel(x, y) = label;
				}
				info._size++;
			} else {
				if (_with_images) {
					comp->pixel(x, y) = 0;
				}
			}
		}
	}

	// that's it
	return info;
}

double	ComponentDecomposer::maxradius() const {
	double	result = 0;
	std::list<ComponentInfo>::const_iterator	ci;
	for (ci = _components.begin(); ci != _components.end(); ci++) {
		double	r = ci->radius();
		if (r > result) {
			result = r;
		}
	}
	return result;
}

} // namespace image
} // namespace image
} // namespace astro
