/*
 * AstroViewer.h -- Viewer
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroViewer_h
#define _AstroViewer_h

#include <AstroImage.h>
#include <AstroPixel.h>
#include <AstroBackground.h>
#include <AstroHistogram.h>
#include <cstdint>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace image {

class ViewerPipeline;

class Viewer {
	ImagePtr	image;
	// pointer for the full image
	typedef	std::shared_ptr<uint32_t>	imagedataptr;
	imagedataptr	_imagedata;
	ImageSize	_displaysize;
public:
	uint32_t	*imagedata() const;
	const ImageSize&	size() const;
	const ImageSize&	displaysize() const;
	void	displaysize(const ImageSize& displaysize);
	void	displayScale(float scale);
	double	displayScale() const;

private:
	// pointer to the previous version of the image
	imagedataptr	_previewdata;
	ImageSize	_previewsize;
public:
	uint32_t	*previewdata() const;
public:
	const ImageSize&	previewsize() const { return _previewsize; }
	void	previewsize(const ImageSize& previewsize);
	void	previewwidth(unsigned int width);

private:
	imagedataptr	_backgrounddata;
	ImageSize	_backgroundsize;
public:
	const ImageSize&	backgroundsize() const { return _backgroundsize; }
	void	backgroundsize(const ImageSize& backgroundsize);
	uint32_t	*backgrounddata() const;

private:
	HistogramSet	_histograms;
public:
	const HistogramSet&	histograms() const { return _histograms; }

private:
	// adapters of the processing pipeline
	ViewerPipeline	*pipeline;
	std::shared_ptr<ViewerPipeline>	pipelineptr;
public:
	Viewer(const std::string& filename);
	~Viewer();

	void	update();
	void	previewupdate();
	void	backgroundupdate();

	// color correction
	RGB<float>	colorcorrection() const;
	void	colorcorrection(const RGB<float>& colorcorrection);

	// background subtraction
	const Background<float>&	background() const;
	void	background(const Background<float>& background);
	bool	backgroundEnabled() const;
	void	backgroundEnabled(bool backgroundsubtract);
	bool	gradientEnabled() const;
	void	gradientEnabled(bool gradientenabled);

	// Gamma correction
	float	gamma() const;
	void	gamma(float gamma);

	// Range extraction
	float	min() const;
	float	max() const;
	void	setRange(float min, float max);

	// change the saturation
	float	saturation() const;
	void	saturation(float saturation);

	void	writeimage(const std::string& filename);

};

} // namespace image
} // namespace astro

#endif /* _AstroViewer_h */
