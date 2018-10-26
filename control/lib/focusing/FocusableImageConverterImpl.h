/*
 * FocusableImageConverter.h -- Class to extract focusable float images
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <typeinfo>
#include <AstroFocus.h>
#include <AstroAdapter.h>
#include <AstroUtils.h>

namespace astro {
namespace focusing {

/**
 * \brief Class to hide implementation details 
 *
 * This class actually implements the image conversion
 */
class FocusableImageConverterImpl : public FocusableImageConverter {
	ImageRectangle	_rectangle;
	ImageRectangle	rectangle_to_use(ImagePtr image);
	FocusableImage	get_raw(ImagePtr image);
	FocusableImage	get_bayer(ImagePtr image);
	FocusableImage	get_yuv(ImagePtr image);
	FocusableImage	get_rgb(ImagePtr image);
public:
	FocusableImageConverterImpl();
	FocusableImageConverterImpl(const ImageRectangle& rectangle);
	virtual ~FocusableImageConverterImpl() { }
	FocusableImage	operator()(ImagePtr image);
};

} // namespace focusing
} // namespace astro
