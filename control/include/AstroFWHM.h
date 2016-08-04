/*
 * AstroFWHM.h -- algorithms for the computation of the FWHM of an image
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <AstroTypes.h>

namespace astro {
namespace image {
namespace fwhm {

class ComponentDecomposer;

/**
 * \brief Component information
 *
 * This class encapsulates the information about a connected component in an
 * image that we need to compute the FWHM. This class has no constructor
 * to set the fields because only the ComponentDecomposer is allowed to
 * set those values.
 */
class ComponentInfo {
	unsigned char	_label;
	Point		_center;
	double		_radius;
	ImagePoint	_representant;
	ImagePtr	_image;
	int		_size;
public:
	unsigned char	label() const { return _label; }
	const Point&	center() const { return _center; }
	double	radius() const { return _radius; }
	const ImagePoint&	representant() const { return _representant; }
	ImagePtr	image() const { return _image; }
	int	size() const { return _size; }
friend class ComponentDecomposer;
	std::string	toString() const;
};

/**
 * \brief Class to decompoes an image into connected components
 *
 * The constructor of this class does all the work, the result is an image
 * containing the component information. The public methods only serve
 * to enable clients to query information about the components
 */
class ComponentDecomposer {
	Image<unsigned char>		_image;
	std::list<ComponentInfo>	_components;
	ComponentInfo	component(unsigned char label) const;
	bool	_with_images;
public:
	ComponentDecomposer(ImagePtr rawimage, bool with_images = false,
		double limit = 0);
	size_t	numberOfComponents() const { return _components.size(); }
	const std::list<ComponentInfo>&	components() const {
		return _components;
	}
	double	maxradius() const;
};




} // namespace fwhm
} // namespace image
} // namespace astro
