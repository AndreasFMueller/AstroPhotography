/*
 * ViewerPipeline.h
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ViewerPipeline_h
#define _ViewerPipeline_h

#include <AstroImage.h>
#include <AstroAdapter.h>
#include <AstroBackground.h>
#include <AstroTonemapping.h>

using namespace astro::adapter;

namespace astro {
namespace image {

class ViewerPipeline : public ConstImageAdapter<unsigned int> {
	//const Image<RGB<float> >		*imagep;
	BackgroundSubtractionAdapter		backgroundsubtract;
	ColorCorrectionAdapter<float>		colorcorrectionadapter;
	LuminanceExtractionAdapter<float>	luminanceimage;
	ColorExtractionAdapter<float>		colorimage;
	RangeAdapter<float>			rangeadapter;
	GammaAdapter<float>			gammaadapter;
	LuminanceScalingAdapter<float>		upscale;
	LuminanceColorAdapter<float>		compose;
	RGB32Adapter<float>			rgb32;
public:
	ViewerPipeline(const Image<RGB<float> > *_imagep);

	virtual const unsigned int	pixel(unsigned int x, unsigned int y) const {
		return (unsigned int)rgb32.pixel(x, y);
	}

	float	gamma() const;
	void	gamma(float gamma);

	float	saturation() const;
	void	saturation(float saturation);

	bool	backgroundEnabled() const;
	void	backgroundEnabled(bool backgroundenabled);

	bool	gradientEnabled() const;
	void	gradientEnabled(bool gradientenabled);

	void	setRange(double min, double max);
	float	max() const;
	float	min() const;

	RGB<float>	colorcorrection() const;
	void	colorcorrection(const RGB<float>& colorcorrection);

	const Background<float>&	background() const;
	void	background(const Background<float>& _background);

	const ConstImageAdapter<RGB<float> >&	processedimage() const;
};

} // namespace image
} // namespace astro

#endif /* _ViewerPipeline_h */
