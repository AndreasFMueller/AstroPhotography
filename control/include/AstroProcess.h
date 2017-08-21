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
#include <AstroUtils.h>

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
	virtual unsigned char	monochrome_pixel(int x, int y) const = 0;
	virtual RGB<unsigned char>	color_pixel(int x, int y) const = 0;
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
	virtual unsigned char	pixel(int x, int y) const {
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
	virtual RGB<unsigned char>	pixel(int x, int y) const {
		return previewadapter->color_pixel(x, y);
	}
};

} // namespace adapter

namespace test {

// we need to declare some test classes so that they can get access
// to the protected methods (otherwise we could only test the classes
// from derived classes, which does not make sense.
class ProcessingStepTest;
class WriteImageFileStepTest;
class ImageBufferStepTest;
class ImageCalibrationStepTest;

} // namespace test

namespace process {

class ProcessingController;
typedef std::shared_ptr<ProcessingController>	ProcessingControllerPtr;
class ProcessingStep;
typedef std::shared_ptr<ProcessingStep>	ProcessingStepPtr;

class ProcessorParser;
class ProcessorNetwork;

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
public:
	static int	newid();
	static void	remember(ProcessingStepPtr step);
	static bool	exists(int id);
	static ProcessingStepPtr	byid(int id);
	static void	forget(int id);
	static bool	inuse(int id);
	static void	checkstate();
	static void	verbose(bool v);
	static bool	verbose();
private:
	// each processing step has an id, and the library ensures that the
	// ids are unique
	int	_id;
public:
	int	id() const { return _id; }
private:
	std::string	_name;
public:
	const std::string&	name() const { return _name; }
	void	name(const std::string& n) { _name = n; }

	// precursors and successors of each step, these turn the processing
	// steps into directed graph
public:	
	typedef	std::list<int>	steps;
private:
	steps	_precursors;
	steps	_successors;
protected:
	const steps&	precursors() const {
		return _precursors;
	}
	const steps&	successors() const {
		return _successors;
	}
public:
	int	precursorCount() const { return _precursors.size(); }
	int	successorCount() const { return _successors.size(); }
	bool	hasSuccessor(int id) const;
	bool	hasPrecursor(int id) const;
	bool	hasSuccessor(ProcessingStepPtr step) const;
	bool	hasPrecursor(ProcessingStepPtr step) const;
protected:
	// derived classes may have their own methods to handle their inputs,
	// but they use the methods here to maintain the dependency graph
	// this methods 
	void	add_precursor(ProcessingStepPtr step);
	void	remove_precursor(ProcessingStepPtr step);
	void	add_successor(ProcessingStepPtr step);
	void	remove_successor(ProcessingStepPtr step);
	friend class ProcessorParser;
private:
	// these methods do not also add the other 
	void	add_precursor(int id);
	void	add_successor(int id);
	void	remove_precursor(int id);
	void	remove_successor(int id);
	// Derived classes may need access to precursors
private:
	friend class ProcessingController;
	// allow test class access
	friend class astro::test::ProcessingStepTest;
	friend class astro::test::WriteImageFileStepTest;
	friend class astro::test::ImageBufferStepTest;
	friend class astro::test::ImageCalibrationStepTest;
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
	 *            has changed. Such process is runnable.
	 * working:   the processing step is currently being worked on by some
	 *            thread
	 * complete:  the work for this processing step is complete
	 * failed:    the work for this processing step failed
	 */
	typedef enum { idle, needswork, working, complete, failed } state; 
static std::string	statename(state s);
private:
	state	precursorstate() const;
	void	checkyourstate();
protected:
	state	_status;
public:
	virtual state	status() const;
	state	status(state newsstate);
protected:
	volatile float	_completion;
public:
	float	completion() const { return _completion; }
	void	work();
	virtual void	cancel();
private:
	astro::thread::Barrier	_barrier;
	friend class ProcessorNetwork;
protected:
	virtual state	do_work();
	// constructor
public:
	ProcessingStep();
	virtual ~ProcessingStep();
private:	// prevent copying
	ProcessingStep(const ProcessingStep& other);
	ProcessingStep&	operator=(const ProcessingStep& other);
public:
	std::string	type_name() const;
#if 0
	virtual bool	hasMetadata(const std::string& name) const;
	virtual Metavalue	getMetadata(const std::string& name) const;
#endif
	// dependency tracking
protected:
	time_t	_when;
	void	when(time_t w) { _when = w; }
public:
	virtual time_t	when() const;
	std::list<int>	unsatisfied_dependencies();

	virtual std::string	what() const = 0;
};

#if 0
/**
 * \brief Processing unit of work is executed by a thread
 *
 * ProcessingThead objects are wrappers around a processing step
 * that handles running the work function in a separate thread.
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
#endif

#if 0
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
#endif

class ImageStep : public ProcessingStep {
protected:
	ImagePtr	_image;
public:
	virtual ImagePtr	image() { return _image; }
	ImageStep() : ProcessingStep() { }
	ImageSequence	precursorimages();
	virtual ProcessingStep::state	do_work() = 0;
};

class FileImageStep : public ImageStep {
	time_t	_lastread;
protected:
	std::string	_filename;
	bool	exists() const;
public:
	FileImageStep(const std::string& filename);
	virtual time_t	when() const;
	virtual ProcessingStep::state	status() const;
	virtual ImagePtr image();
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

class WriteableFileImageStep : public FileImageStep {
public:
	WriteableFileImageStep(const std::string& filename);
private:
	virtual ProcessingStep::state	do_work();
	virtual ProcessingStep::state	status() const;
	virtual std::string	what() const;
};

class DarkImageStep : public ImageStep {
public:
	DarkImageStep();
private:
	double	_badpixellimit;
public:
	double	badpixellimit() const { return _badpixellimit; }
	void	badpixellimit(double b) { _badpixellimit = b; }
private:
	virtual ProcessingStep::state	do_work();
public:
	virtual std::string	what() const;
};

class FlatImageStep : public ImageStep {
	ProcessingStepPtr	_dark;
public:
	ProcessingStepPtr	dark() const { return _dark; }
	void	dark(ProcessingStepPtr d) { _dark = d; }
	FlatImageStep();
private:
	virtual ProcessingStep::state	do_work();
public:
	virtual std::string	what() const;
};

class ImageCalibrationStep : public ImageStep {
	ProcessingStepPtr	_dark;
	ProcessingStepPtr	_flat;
	bool	_interpolate;
	bool	_demosaic;
	bool	_flip;
public:
	ProcessingStepPtr	dark() const { return _dark; }
	void	dark(ProcessingStepPtr d) { _dark = d; }
	ProcessingStepPtr	flat() const { return _flat; }
	void	flat(ProcessingStepPtr f) { _flat = f; }
	bool	interpolate() const { return _interpolate; }
	void	interpolate(bool i) { _interpolate = i; }
	bool	demosaic() const { return _demosaic; }
	void	demosaic(bool d) { _demosaic = d; }
	bool	flip() const { return _flip; }
	void	flip(bool f) { _flip = f; }
	ImageCalibrationStep();
	virtual ProcessingStep::state	do_work();
	virtual ProcessingStep::state	status() const;
	virtual std::string	what() const;
};

#if 0
/**
 * \brief Image Steps also have image output and preview
 *
 * Image Steps are Processing steps that have an output method
 * and a preview adapter.
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
protected:
	ImageStep	*input() const;
public:
	// metadata
	virtual bool	hasMetadata(const std::string& name) const;
	virtual astro::image::Metavalue	getMetadata(const std::string& name) const;
	virtual ImageMetadata::const_iterator	begin() const;
	virtual ImageMetadata::const_iterator	end() const;
};

/**
 * \brief Using a raw image as input
 */
class RawImageStep : public ImageStep {
protected:
	ImagePtr	_image;
	ProcessingStep::state	common_work();
public:
	RawImageStep(ImagePtr image);
	ImageRectangle	subframe() const;
	virtual ProcessingStep::state	do_work();
	// metadata
	virtual bool	hasMetadata(const std::string& name) const;
	virtual astro::image::Metavalue	getMetadata(const std::string& name) const;
	virtual ImageMetadata::const_iterator	begin() const;
	virtual ImageMetadata::const_iterator	end() const;
};

/**
 * \brief Reading a Disk file
 *
 * The input for all processing are files stored on disk. This class
 * gives access to such files, and allows to preview them.
 */
class RawImageFileStep : public RawImageStep {
	std::string	_filename;
public:
	RawImageFileStep(const std::string& filename);
	virtual ~RawImageFileStep();
	// the actual work function has to read the image, and has to
	// construct the preview adapter
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Create an image from the input and keep a copy of it in memory
 *
 * This type of processin step ensures that recomputations are keept to
 * a minimum. This type of step should be included whenever lots of access
 * to the same pixel by the successors are expected.
 */
class ImageBufferStep : public ImageStep {
	Image<double>	*image;
	ImagePtr	imageptr;
public:
	ImageBufferStep();
	virtual ProcessingStep::state	do_work();
	virtual const ConstImageAdapter<double>&	out() const;
	// metadata
	virtual bool	hasMetadata(const std::string& name) const;
	virtual astro::image::Metavalue	getMetadata(const std::string& name) const;
	virtual ImageMetadata::const_iterator	begin() const;
	virtual ImageMetadata::const_iterator	end() const;
};

/**
 * \brief Write an image to a disk file
 */
class WriteImageStep : public ImageStep {
	std::string	_filename;
	bool	_precious;
public:
	WriteImageStep(const std::string& filename, bool precious = false);
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
class CalibrationImageStep : public ImageStep {
public:
	typedef enum { DARK, FLAT } caltype;
protected:
	caltype	_type;
public:
	caltype	type() const { return _type; }
static	std::string	caltypename(caltype t);
protected:
	ImagePtr	_image;
	virtual ProcessingStep::state	do_work();
public:
	CalibrationImageStep(caltype t) : _type(t) { }
	CalibrationImageStep(caltype t, ImagePtr image);
	// metadata
	virtual bool	hasMetadata(const std::string& name) const;
	virtual astro::image::Metavalue	getMetadata(const std::string& name) const;
};

/**
 * \brief Calibration image read from a file
 */
class CalibrationImageFileStep : public CalibrationImageStep {
	std::string	_filename;
public:
	CalibrationImageFileStep(const std::string& filename,
		const CalibrationImageStep::caltype type)
			: CalibrationImageStep(type), _filename(filename) {
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
class ImageCalibrationStep : public ImageStep {
public:
	const CalibrationImageStep	*calimage(CalibrationImageStep::caltype) const;
	ImageCalibrationStep();
	virtual ~ImageCalibrationStep();
	// there is no work to, as calibration can be done on the fly
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Common methods for calbration image generators
 *
 * The calibration image processor computes calibration image pixel values
 * from the averages computed in tiles. The tiles are centered at coordinates
 * that are odd multiples of _step.
 *
 * The image may have non-comparable pixels. E.g. a color sensor has color
 * filters on each pixel, so the R, G, and B pixels are different, and must
 * be treated separately to compute the calibration image. Pixels of the same
 * color form a subgrid, the _spacing parameter is the grid constant of the
 * subgrid. Monochrome images, where all pixels are comparable, use
 * _spacing = 1. Bayer filter sensors use _spacing = 2. To ensure that the
 * tile center coordinates always belong th the same subgrid, _step must be
 * a multiple of _spacing. Note that the _spacing parameter has no influence
 * on the position of the tile centers or the size of the tiles, it only affects
 * the size of the target image (it multiplies each dimension by _spacing)
 * and the number of pixels that contribute to a tile aggregate (it devidides
 * it by _spacing).
 *
 * To make the code more readable, this class always uses short to represent
 * tile coordinates.
 */
// The dots represent the area of the image that used to compute the aggregates
// for the tile with coordinates (1,1).
//
//         |         |         |         |         |         |
//         |         |       ..|.........|.........|..       |
// 4*_step +---------+---------+---------+---------+---------+-----
//         |         |       ..|.........|.........|..       |
//         |         |       ..|.........|.........|..       |
//         |         |       ..|.........|.........|..       |
// 3*_step +---------o---------+---------o---------+---------o-----
//         |         |       ..|.........|.........|..       |
//         |         |       ..|.........|.........|..       |
//         |         |       ..|.........|.........|..       |
// 2*_step +---------+---------+---------+---------+---------+-----
//         |         |       ..|.........|.........|..       |
//         |         |         |         |         |         |
//         |         |         |         |         |         |
//   _step +---------o---------+---------o---------+---------o-----
//         |         |         |         |         |         |
//         |         |         |         |         |         |
//         |         |         |         |         |         |
//       0 +---------+---------+---------+---------+---------+-----
//         0        _step   2*_step   3*_step   4*_step   5*_step
//
class CalibrationProcessorStep : public CalibrationImageStep {
	int	_spacing;
public:
	int	spacing() const { return _spacing; }
	void	spacing(int s);
private:
	int	_step;
public:
	int	step() const { return _step; }
	void	step(int s);
	void	setStepAndSpacing(int newstep, int newspacing);
private:
	double	_tolerance;
public:
	double	tolerance() const { return _tolerance; }
	void	tolerance(double t);
private:
	double	_maxoffset;
public:
	double	maxoffset() const { return _maxoffset; }
	void	maxoffset(double m) { _maxoffset = m; }
private:
	double	_margin;
public:
	double	margin() const { return _margin; }
	void	margin(double m) { _margin = m; }

	/**
 	 * \brief Method type 
	 * 
	 * constructing a dark or flat images involves deciding which pixel
	 * values should be considered when computing a new pixel value.
	 * 
	 */
	typedef enum { mean_method, median_method } method_type;
private:
	method_type	_method;
public:
	method_type	method() const { return _method; }
	void	method(method_type m) { _method = m; }
	
protected:
	int	grid() const { return _step * _spacing; }
	size_t		nrawimages;
	ImageStep	**rawimages;
	size_t	getPrecursors();

	// conversion of image coordinates to tile coordinates
	// tile center coordinates from image coordinates 
	int	xc(int x) const;
	int	yc(int y) const;
	// image coordinates from tile coordinates
	int	xi(short x) const;
	int	yi(short y) const;
	// tile coordinates from image coordinates
	short	xt(int x) const;
	short	yt(int y) const;

	ImageSize	tileimagesize(const ImageSize& size) const;

	// aggregates class
public:
	class aggregates {
	public:
		double	mean;
		double	median;
		double	stddev;
		bool	improbable(double x, double tolerance = 3.) const {
			return (fabs(x - mean) > tolerance * stddev);
		}
	};
protected:
	Image<double>	*image;
	ImagePtr	imageptr;
private:
	void	copy_common_metadata(const std::string& name);
public:
	// metadata
	virtual bool	hasMetadata(const std::string& name) const;
	virtual astro::image::Metavalue	getMetadata(const std::string& name) const;
protected:
	// as a basis for deciding which values should go into the
	// construction of the calibration image, we compute medians,
	// means and standard deviations. 
	Image<double>	*medians;
	Image<double>	*means;
	Image<double>	*stddevs;
private:
	aggregates	tile(int x, int y);
	void	filltile(short x, short y);
	aggregates	aggr(int x, int y) const;
public:
	CalibrationProcessorStep(CalibrationImageStep::caltype t);
	~CalibrationProcessorStep();
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
class DarkProcessorStep : public CalibrationProcessorStep {
public:
	DarkProcessorStep()
		: CalibrationProcessorStep(CalibrationImageStep::DARK) { }
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Processor to create a flat image from a set of inputs
 */
class FlatProcessorStep : public CalibrationProcessorStep {
public:
	FlatProcessorStep()
		: CalibrationProcessorStep(CalibrationImageStep::FLAT) { }
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief Processing step to interpolate pixels
 */
class InterpolationStep : public ImageStep {
	int	_spacing;
public:
	InterpolationStep(int spacing = 1);
	virtual ProcessingStep::state	do_work();
};

/**
 * \brief
 */
class RGBDemosaicingStep : public ImageStep {
public:
};
#endif

/**
 * \brief Network Class to manage a complete network of interdependen steps
 */
class ProcessorNetwork {
	typedef std::map<int, ProcessingStepPtr>	stepmap_t;
	typedef std::map<int, std::string>		id2namemap_t;
	typedef std::multimap<std::string, int>		name2idmap_t;

	stepmap_t	_steps;
	id2namemap_t	_id2names;
	name2idmap_t	_name2ids;
public:
	ProcessorNetwork();
	void	add(ProcessingStepPtr step);
	ProcessingStepPtr	byid(int id) const;
	ProcessingStepPtr	byname(const std::string& name) const;
	ProcessingStepPtr	bynameid(const std::string& name) const;
	ProcessingStep::steps	terminals() const;
	ProcessingStep::steps	initials() const;
private:
	int	_maxthreads;
public:
	int	maxthreads() const { return _maxthreads; }
	void	maxthreads(int m) { _maxthreads = m; }
private:
	typedef std::shared_ptr<std::thread>	thread_ptr;
	std::vector<thread_ptr>	_threads;
public:
	void	checkstate();
	bool	hasneedswork();
	void	process();
	int	process(int id);
	int	process(const ProcessingStep::steps& steps);
};
typedef std::shared_ptr<ProcessorNetwork>	ProcessorNetworkPtr;

/**
 * \brief Factory class to build processing networks from files or strings
 */
class ProcessorFactory {
public:
	ProcessorFactory();
	ProcessorNetworkPtr	operator()(void);
	ProcessorNetworkPtr	operator()(const std::string& filename);
	ProcessorNetworkPtr	operator()(const char *data, int size);
};

} // namespace process
} // namespace astro

#endif /* _AstroProcess_h */
