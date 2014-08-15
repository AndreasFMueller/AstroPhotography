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
 *
 * All processing steps allow preview of the product of this step. If the
 * steps are just simple pixel operations, then this class is essentially
 * an adapter. For more complicated steps, the step may need to store
 * the intermediate image. Then this class will be an adapter to the
 * intermediate image.
 *
 * Preview is always limited to 8 bit, because that is what modern computer
 * screens can display. The preview does not need to be processed. But it
 * does not make sense to convert an image completely to 8 bit, a small size
 * preview me only sample a subgrid of the image. Again, the adapter approach
 * may save a lot of work.
 *
 * Displaying an image with deep pixel size as unsigned char images is only
 * possible if the pixel values can reasonably be rescaled. This is the
 * purpose of the _min and _max member variables. The application can set them
 * so that a reasonable image may be displayed. 
 *
 * Due to the range of possible inputs for this adapter, this is just an
 * abstract base class. Implementation classes are expected to implement
 * the size method and the two pixel methods. The class is also a factory
 * class, the static get methods return a previewadapter from an image or
 * an ImagePtr. The get method does not take ownership of the pointer, 
 * as preview is just another function added to a an image. It is expected
 * that the processing step classes will own the image.
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

/**
 * \brief ProcessingStep base class
 *
 * All image processing steps are of this type. Each processing step has an
 * interface to access a preview. If the step takes time, and cannot be
 * completed pixel by pixel, then the work method will take some time
 * to complete, it is probably a good idea to run it in a separate thread.
 * 
 */
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

	// The processing step has at least one output, which must be an
	// image. The processing may have some byproducts, but they are
 	// processing step dependen
public:
	virtual const ConstImageAdapter<double>&	out() const;
	virtual bool	hasColor() const;
	virtual const ConstImageAdapter<RGB<double> >&	out_color() const;
};

/**
 * \brief Reading a Disk file
 *
 * The input for all processing are files stored on disk. This class
 * gives access to such files, and allows to preview them.
 */
class ImageFile : public ProcessingStep {
	ImagePtr	_image;
public:
	ImageFile();
	
};

/**
 * \brief Raw images from a CCD camera must be calibrated by this class
 *
 * This class takes a dark and a flat image reference and calibrates
 * any image. The result image will be use double pixel format, but the
 * original image will typicall be unsigned char or unsigned short. The
 * flat and the dark will both be float images.
 */
class ImageCalibration : public ProcessingStep {
	ImagePtr	_dark;
	ImagePtr	_flat;
	const ConstImageAdapter<double>&	image;
public:
	ImageCalibration(const ConstImageAdapter<double>&,
		ImagePtr _dark, ImagePtr _flat);
};

/**
 * \brief Processor to create a dark image from a set of inputs
 */
class DarkProcessor {
public:
};

/**
 * \brief Processor to create a flat image from a set of inputs
 */
class LightProcessor {
public:
};

/**
 * \brief
 */
class {
};

} // namespace process
} // namespace astro

#endif /* _AstroProcess_h */
