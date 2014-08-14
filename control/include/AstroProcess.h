/*
 * AstroProcess.h - Abstrations for image processing in astrophotography
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroProcess_h
#define _AstroProcess_h

#include <memory>
#include <AstroImage.h>
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

/**
 * \brief Adapter fro Previewing 
 */
class PreviewAdapter {
protected:
	double	_min, _max;
public:
	double	min() const { return _min; }
	void	min(double m) { _min = m; }
	double	max() const { return _max; }
	void	max(double m) { _max = m; }
	virtual ImageSize	size() const = 0;
	virtual unsigned char	monochrome_pixel(unsigned int x,
					unsigned int y) const = 0;
	virtual RGB<unsigned char>	color_pixel(unsigned int x,
						unsigned int y) const = 0;
private:
	PreviewAdapter(const PreviewAdapter& other);
	PreviewAdapter&	operator=(const PreviewAdapter& other);
public:
	PreviewAdapter() { _min = 0.; _max = 1.; }
	virtual ~PreviewAdapter();
static PreviewAdapter	*get(const ImageBase *image);
static PreviewAdapter	*get(ImagePtr image);
};


/**
 * \brief Adapter to return unsigned char images
 */
class PreviewMonochromeAdapter : public ConstImageAdapter<unsigned char> {
	const PreviewAdapter&	previewadapter;
public:
	PreviewMonochromeAdapter(const PreviewAdapter& _previewadapter)
		: ConstImageAdapter<unsigned char>(_previewadapter.size()),
		  previewadapter(_previewadapter) { }
	virtual unsigned char	pixel(unsigned int x, unsigned int y) const {
		return previewadapter.monochrome_pixel(x, y);
	}
};

/**
 * \brief Adapter to return unsigned char RGB images
 */
class PreviewColorAdapter : public ConstImageAdapter<RGB<unsigned char> > {
	const PreviewAdapter&	previewadapter;
public:
	PreviewColorAdapter(const PreviewAdapter& _previewadapter)
		: ConstImageAdapter<RGB<unsigned char> >(_previewadapter.size()),
		  previewadapter(_previewadapter) { }
	virtual RGB<unsigned char>	pixel(unsigned int x,
						unsigned int y) const {
		return previewadapter.color_pixel(x, y);
	}
};

} // namespace adapter

namespace process {

class ProcessingStep;
typedef std::shared_ptr<ProcessingStep>	ProcessingStepPtr;

class ProcessingStep {
private:
	astro::adapter::PreviewAdapter	*preview;
public:
	astro::adapter::PreviewMonochromeAdapter	monochrome_preview();
	astro::adapter::PreviewColorAdapter	color_preview();

	// Do the actual processing
private:
	volatile float	_completion;
public:
	float	completion() const { return _completion; }
	bool	completed() const { return _completion >= 1.; }
	void	work();

	// constructor
public:
	ProcessingStep();
	virtual ~ProcessingStep();
private:	// prevent copying
	ProcessingStep(const ProcessingStep& other);
	ProcessingStep&	operator=(const ProcessingStep& other);
};

class ImageFile {
	ImagePtr	_image;
public:
	ImageFile();
	
};

} // namespace process
} // namespace astro

#endif /* _AstroProcess_h */
