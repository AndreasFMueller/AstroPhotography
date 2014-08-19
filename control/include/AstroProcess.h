/*
 * AstroProcess.h - Abstrations for image processing in astrophotography
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroProcess_h
#define _AstroProcess_h

#include <memory>
#include <list>
#include <AstroImage.h>
#include <AstroAdapter.h>

namespace astro {
namespace adapter {

class PreviewAdapter;
typedef std::shared_ptr<PreviewAdapter>	PreviewAdapterPtr;
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
static PreviewAdapterPtr	get(const ImageBase *image);
static PreviewAdapterPtr	get(ImagePtr image);
};

/**
 * \brief Adapter to return unsigned char images
 */
class PreviewMonochromeAdapter : public ConstImageAdapter<unsigned char> {
	const PreviewAdapterPtr	previewadapter;
public:
	PreviewMonochromeAdapter(const PreviewAdapterPtr _previewadapter)
		: ConstImageAdapter<unsigned char>(_previewadapter->size()),
		  previewadapter(_previewadapter) { }
	virtual unsigned char	pixel(unsigned int x, unsigned int y) const {
		return previewadapter->monochrome_pixel(x, y);
	}
};

/**
 * \brief Adapter to return unsigned char RGB images
 */
class PreviewColorAdapter : public ConstImageAdapter<RGB<unsigned char> > {
	const PreviewAdapterPtr	previewadapter;
public:
	PreviewColorAdapter(const PreviewAdapterPtr _previewadapter)
		: ConstImageAdapter<RGB<unsigned char> >(_previewadapter->size()),
		  previewadapter(_previewadapter) { }
	virtual RGB<unsigned char>	pixel(unsigned int x,
						unsigned int y) const {
		return previewadapter->color_pixel(x, y);
	}
};

} // namespace adapter

namespace test {

class ProcessingStepTest;
class WriteImageFileStepTest;

} // namespace test

namespace process {

class ProcessingController;
typedef std::shared_ptr<ProcessingController>	ProcessingControllerPtr;
class ProcessingStep;
typedef std::shared_ptr<ProcessingStep>	ProcessingStepPtr;
class ProcessingThread;
typedef std::shared_ptr<ProcessingThread>	ProcessingThreadPtr;

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
	// precursors and successors of each step, these turn the processing
	// steps into directed graph
public:	
	typedef	std::list<ProcessingStep *>	steps;
private:
	steps	_precursors;
	steps	_successors;
protected:
	const steps	precursors() const {
		return _precursors;
	}
	const steps	successors() const {
		return _successors;
	}
protected:
	// derived classes may have their own methods to handle their inputs,
	// but they use the methods here to maintain the dependency graph
	void	add_precursor(ProcessingStep *step);
	void	remove_precursor(ProcessingStep *step);
	void	add_successor(ProcessingStep *step);
	void	remove_successor(ProcessingStep *step);
	// Derived classes may need access to precursors
private:
	void	add_precursor(ProcessingStepPtr step);
	void	remove_precursor(ProcessingStepPtr step);
	void	add_successor(ProcessingStepPtr step);
	void	remove_successor(ProcessingStepPtr step);
	friend class ProcessingController;
	// allow test class access
	friend class astro::test::ProcessingStepTest;
	friend class astro::test::WriteImageFileStepTest;
public:
	void	remove_me();

	// Do the actual processing
public:
	/**
	 * State of a processing step:
	 * idle:      the processing step is currently not completely configured
	 *            it cannot even decide whether it needs work. This is the
	 *            default state in which all processing steps are created.
	 * needswork: the processing step is completely configured, but
	 *            it needs to work before it can provide any results.
	 *            This state is also reached when the state of a previous
	 *            has changed.
	 * working:   the processing step is current being worked on by some
	 *            thread
	 * complete:  the work for this processing step is complete
	 */
	typedef enum { idle, needswork, working, complete } state; 
static std::string	statename(state s);
private:
	state	precursorstate() const;
protected:
	state	_status;
	volatile float	_completion;
public:
	float	completion() const { return _completion; }
	state	status() const { return _status; }
	state	status(state newsstate);
	virtual state	checkstate();
	void	work(ProcessingThread *thread = NULL);
	virtual void	cancel();
protected:
	virtual state	do_work();
	// constructor
public:
	ProcessingStep();
	virtual ~ProcessingStep();
private:	// prevent copying
	ProcessingStep(const ProcessingStep& other);
	ProcessingStep&	operator=(const ProcessingStep& other);
};

/**
 * \brief Image Steps also have image output and preview
 */
class ImageStep : public ProcessingStep {
	// constructor
public:
	ImageStep();
	virtual ~ImageStep();

	// access to the preview for this processing step. The preview
	// pointer is protected so that derived classes can assign a 
	// suitable preview class.
protected:
	astro::adapter::PreviewAdapterPtr	_preview;
public:
	virtual astro::adapter::PreviewAdapterPtr	preview() const {
		return _preview;
	}
	astro::adapter::PreviewMonochromeAdapter	monochrome_preview();
	astro::adapter::PreviewColorAdapter	color_preview();

	// The processing step has at least one output, which must be an
	// image. The processing may have some byproducts, but they are
 	// processing step dependen
protected:
	typedef std::shared_ptr<ConstImageAdapter<double> >	outPtr;
	outPtr	_out;
public:
	virtual const ConstImageAdapter<double>&	out() const;
	virtual bool	hasColor() const;
	virtual const ConstImageAdapter<RGB<double> >&	out_color() const;
};

/**
 * \brief Processing unit of work is executed by a thread
 */
class ProcessingThread {
protected:
	ProcessingStepPtr	_step;
public:
	ProcessingStepPtr	step() { return _step; }
protected:
	ProcessingThread(ProcessingStepPtr step) : _step(step) { }
	virtual ~ProcessingThread();
public:
	virtual void	cancel() = 0;
	virtual void	wait() = 0;
	virtual void	run(int fd = -1) = 0;
	virtual bool	isrunning() = 0;
	virtual void	started() = 0;
	ProcessingStep::state	status() const { return _step->status(); }
static ProcessingThreadPtr	get(ProcessingStepPtr step);
};

/**
 * \brief Manage a network of processing steps
 *
 * The Processing controller allows to manipulate a network of processing
 * steps by name, and controls the execution of the work to be done, possibly
 * in multiple threads.
 */
class ProcessingController {
	typedef	std::map<std::string, ProcessingThreadPtr>	stepmap;
	stepmap	steps;
public:
	ProcessingController();
	~ProcessingController();

	// adding and removing steps
	void	addstep(const std::string& name, ProcessingStepPtr step);
	void	removestep(const std::string& name);
	void	removestep(ProcessingStepPtr step);

	// network maintenance
	void	add_precursor(const std::string& target,
			const std::string& precursor);
	void	remove_precursor(const std::string& target,
			const std::string& precursor);
	void	add_successor(const std::string& target,
			const std::string& successor);
	void	remove_successor(const std::string& target,
			const std::string& successor);

	// locating steps 
	ProcessingStepPtr	find(const std::string& name);
	std::string	name(ProcessingStepPtr step);

	// execute all the necessary steps
	bool	haswork();
	void	execute(size_t nthreads = 1);
private:
	stepmap::iterator	stepneedingwork();
};

/**
 * \brief Using a raw image as input
 */
class RawImage : public ImageStep {
protected:
	ImagePtr	_image;
	ProcessingStep::state	common_work();
public:
	RawImage(ImagePtr image);
	ImageRectangle	subframe() const;
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Reading a Disk file
 *
 * The input for all processing are files stored on disk. This class
 * gives access to such files, and allows to preview them.
 */
class RawImageFile : public RawImage {
	std::string	_filename;
public:
	RawImageFile(const std::string& filename);
	virtual ~RawImageFile();
	// the actual work function has to read the image, and has to
	// construct the preview adapter
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Write an image to a disk file
 */
class WriteImage : public ImageStep {
	std::string	_filename;
	bool	_precious;
	ImageStep	*input() const;
public:
	WriteImage(const std::string& filename, bool precious = false);
	virtual ProcessingStep::state	do_work();
	virtual astro::adapter::PreviewAdapterPtr	preview() const;
	virtual const ConstImageAdapter<double>&	out() const;
};

/**
 * \brief Calibration Image steps
 *
 * A Calibration image can be read from a file, or it can be created on
 * the fly
 */
class CalibrationImage : public ImageStep {
public:
	typedef enum { DARK, FLAT } caltype;
protected:
	caltype	_type;
public:
	caltype	type() const { return _type; }
	CalibrationImage(caltype t) : _type(t) { }
static	std::string	caltypename(caltype t);
};

/**
 * \brief Calibration image read from a file
 */
class CalibrationImageFile : public CalibrationImage {
	std::string	_filename;
	ImagePtr	_image;
public:
	CalibrationImageFile(const std::string& filename,
		const CalibrationImage::caltype type)
			: CalibrationImage(type), _filename(filename) {
	}
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Raw images from a CCD camera must be calibrated by this class
 *
 * This class takes a dark and a flat image reference and calibrates
 * any image. The result image will be use double pixel format, but the
 * original image will typicall be unsigned char or unsigned short. The
 * flat and the dark will both be float images.
 */
class ImageCalibration : public ImageStep {
	const ConstImageAdapter<double>	*_image;
	const CalibrationImage	*calimage(CalibrationImage::caltype) const;
public:
	ImageCalibration();
	virtual ~ImageCalibration();
	// there is no work to, as calibration can be done on the fly
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Common methods for calbration image generators
 */
class CalibrationProcessor : public CalibrationImage {
protected:
	size_t		nrawimages;
	ImageStep	**rawimages;
	size_t	getPrecursors();
	void	get(unsigned int x, unsigned int y, double *values, int& n);
	Image<double>	*image;
	ImagePtr	imageptr;
public:
	CalibrationProcessor(CalibrationImage::caltype t);
	~CalibrationProcessor();
	virtual ProcessingStep::state	do_work() = 0;
	// this ensures that the CalibrationProcessor cannot be instantiated
	// directly, only its derived classes can
	virtual const ConstImageAdapter<double>&	out() const;
protected:
	ProcessingStep::state	common_work();
};

/**
 * \brief Processor to create a dark image from a set of inputs
 */
class DarkProcessor : public CalibrationProcessor {
public:
	DarkProcessor() : CalibrationProcessor(CalibrationImage::DARK) { }
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Processor to create a flat image from a set of inputs
 */
class FlatProcessor : public CalibrationProcessor {
public:
	FlatProcessor() : CalibrationProcessor(CalibrationImage::FLAT) { }
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief
 */
class RGBDemosaicing : public ImageStep {
public:
};

} // namespace process
} // namespace astro

#endif /* _AstroProcess_h */
