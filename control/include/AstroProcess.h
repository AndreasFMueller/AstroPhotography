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
#include <AstroPostprocessing.h>
#include <AstroTonemapping.h>
#include <AstroTransform.h>
#include <AstroCamera.h>
#include <thread>
#include <mutex>
#include <iostream>

namespace astro {

namespace process {

class StepPath;
typedef std::shared_ptr<StepPath>	StepPathPtr;

/**
 * \brief Class representing the path of a step
 *
 * This class is designed so that hierarchical paths are easy to formulate
 * in the process XML file. A StepPath object always represents a directory,
 * never a file.
 */
class StepPath {
	// that _path member contains the path valid for the current step.
	// If that path begins with a /, it is to be interpreted as a
	// absolute path, if it begins with a !, the ! should be stripped
	// it should be considered a path relativ to the parent path
	// in all other cases, it is to be considered a path relative
	// to the current working directory.
	std::string	_path;
	bool	absolute(const std::string& s) const;
	bool	relative(const std::string& s) const;
	bool	parent_relative(const std::string& s) const;
public:
	StepPath(StepPathPtr parent = StepPathPtr());
	StepPath(const std::string& p, StepPathPtr parent = StepPathPtr());
	const std::string&	path() const { return _path; }
	std::string	dir() const;
	std::string	file(const std::string& f) const;
	bool	direxists() const;
	bool	fileexists(const std::string& f) const;
};

} // namespace process

namespace adapter {

class PreviewAdapter;
typedef std::shared_ptr<PreviewAdapter>	PreviewAdapterPtr;
/**
 * \brief Adapter for Previewing 
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
class ImageCalibrationStepTest;

} // namespace test

namespace process {

class ProcessingStep;
typedef std::shared_ptr<ProcessingStep>	ProcessingStepPtr;

class ProcessorParser;
class ProcessorNetwork;
class ProcessingThread;

class ProcessingSteps;

/**
 * \brief Object keeping common node information
 */
class NodePaths {
protected:
	StepPathPtr	_srcpath;
	StepPathPtr	_dstpath;
public:
	NodePaths();
	NodePaths(NodePaths& other);
	std::string	srcfile(const std::string& file) const;
	std::string	dstfile(const std::string& file) const;
	StepPathPtr	srcpath() const { return _srcpath; }
	StepPathPtr	dstpath() const { return _dstpath; }
	virtual std::string	info() const;
	friend class ProcessorParser;
};

/**
 * \brief ProcessingStep base class
 *
 * All image processing steps are of this type. Each processing step has an
 * interface to access a preview. If the step takes time, and cannot be
 * completed pixel by pixel, then the work method will take some time
 * to complete, it is probably a good idea to run it in a separate thread.
 * 
 */
class ProcessingStep : public NodePaths {
	static ProcessingSteps	*processing_steps;
public:
	static int	newid();
	static void	remember(ProcessingStepPtr step);
	static void	forget(int id);
	static bool	exists(int id);
	static ProcessingStepPtr	byid(int id);
	static bool	inuse(int id);
	static void	verbose(bool v);
	static bool	verbose();
	static void	clear();
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

	virtual std::string	info() const;
	virtual std::string	verboseinfo() const;
private:
	double	_weight;
	transform::Transform	_transform;
public:
	double	weight() const { return _weight; }
	void	weight(double w) { _weight = w; }
	const transform::Transform&	transform() const { return _transform; }
	void	transform(const transform::Transform& t) { _transform = t; }

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
	void	dumpSuccessors(std::ostream& out) const;
	void	dumpPrecursors(std::ostream& out) const;
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
	// allow test class access
	friend class astro::test::ProcessingStepTest;
	friend class astro::test::WriteImageFileStepTest;
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
private:
	state	_status;
public:
	virtual state	status();
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
	friend class ProcessingThread;
protected:
	virtual state	do_work();
	// constructor
public:
	ProcessingStep();
	ProcessingStep(NodePaths& parent);
	virtual ~ProcessingStep();
	ProcessingStep(const ProcessingStep& other) = delete;
	ProcessingStep&	operator=(const ProcessingStep& other) = delete;
public:
	std::string	type_name() const;
	// dependency tracking
protected:
	time_t	_when;
	void	when(time_t w) { _when = w; }
public:
	virtual time_t	when() const;
	std::list<int>	unsatisfied_dependencies();

	virtual std::string	what() const = 0;
};

/**
 * \brief A step used for grouping in the configuration file only
 */
class GroupStep : public ProcessingStep {
public:
	GroupStep(NodePaths& parent) : ProcessingStep(parent) { }
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Thread performing the processing work within a step
 */
class ProcessingThread : public std::thread {
	ProcessingStepPtr	_step;
public:
	ProcessingStepPtr	step() const { return _step; }
	ProcessingThread(ProcessingStepPtr step);
	~ProcessingThread();
	void	work();
};
typedef std::shared_ptr<ProcessingThread>	ProcessingThreadPtr;

/**
 * \brief Base class for steps that process an image
 */
class ImageStep : public ProcessingStep {
protected:
	ImagePtr	_image;
public:
	virtual ImagePtr	image() { return _image; }
	ImageStep(NodePaths& parent) : ProcessingStep(parent) { }
	ImageSequence	precursorimages(std::vector<int> exlude
				= std::vector<int>()) const;
	bool	precursorSizesConsistent(std::vector<int> exlude
				= std::vector<int>()) const;
	virtual ProcessingStep::state	do_work() = 0;
	ImagePtr	precursorimage(std::vector<int> exlude
				= std::vector<int>()) const;
};

/**
 * \brief Image step to extract a plane from an image
 */
class ImagePlaneStep : public ImageStep {
	int	_n;
public:
	int	n() const { return _n; }
	void	n(int i) { _n = i; }
	ImagePlaneStep(NodePaths& parent) : ImageStep(parent), _n(0) { }
	ImagePlaneStep(NodePaths& parent, int n) : ImageStep(parent), _n(n) { }
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief File Image step class definition
 *
 * A processing step that is based on a file
 */
class FileImageStep : public ImageStep {
protected:
	std::string	_filename;
public:
	const std::string&	filename() const { return _filename; }
	void	filename(const std::string& f) { _filename = f; }
	std::string	srcname() const;
	std::string	dstname() const;
	virtual std::string	fullname() const;
protected:
	bool	_exists;
	bool	exists();
public:
	FileImageStep(NodePaths& parent, const std::string& filename);
	~FileImageStep();
	virtual time_t	when() const;
	virtual ProcessingStep::state	status();
	virtual ImagePtr image();
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
	virtual std::string	verboseinfo() const;
};

/**
 * \brief Writable File image step
 *
 * The writeable file image step writes an image to a file when its
 * dependencies have changed
 */
class WritableFileImageStep : public FileImageStep {
	std::recursive_mutex	_mutex;
public:
	WritableFileImageStep(NodePaths& parent, const std::string& filename);
	virtual std::string	fullname() const;
private:
	virtual ProcessingStep::state	do_work();
	ProcessingStep::state	_previousstate;
	virtual ProcessingStep::state	status();
	virtual std::string	what() const;
	virtual ImagePtr	image();
};

/**
 * \brief Dark image step
 *
 * The dark image step constructs a dark image from a set of dark images
 */
class DarkImageStep : public ImageStep {
	camera::Exposure::purpose_t	_purpose;
public:
	camera::Exposure::purpose_t	purpose() const { return _purpose; }
public:
	DarkImageStep(NodePaths& parent,
		camera::Exposure::purpose_t purpose = camera::Exposure::flat);
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

/** 
 * \brief Flat Image step
 *
 * The flat image step constructs a flat image from a set of precursor
 * images.
 */
class FlatImageStep : public ImageStep {
	ProcessingStepPtr	_dark;
	bool	_mosaic;
public:
	ProcessingStepPtr	dark() const { return _dark; }
	void	dark(ProcessingStepPtr d) { _dark = d; }
	bool	mosaic() const { return _mosaic; }
	void	mosaic(const bool m) { _mosaic = m; }
	FlatImageStep(NodePaths& parent);
private:
	virtual ProcessingStep::state	do_work();
public:
	virtual std::string	what() const;
};

/**
 * \brief image calibration step
 *
 * The image calibration step applies dark and flat images to a an input
 * image.
 */
class ImageCalibrationStep : public ImageStep {
	ProcessingStepPtr	_dark;
	ProcessingStepPtr	_flat;
	bool	_interpolate;
	bool	_demosaic;
	bool	_flip;
	bool	_hflip;
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
	bool	hflip() const { return _hflip; }
	void	hflip(bool f) { _hflip = f; }
	ImageCalibrationStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual ProcessingStep::state	status();
	virtual std::string	what() const;
};

/**
 * \brief Some image processing operations
 */
class ImageTransformationStep : public ImageStep {
	bool	_vertical_flip;
	bool	_horizontal_flip;
	int	_scale;
	double	_xshift;
	double	_yshift;
public:
	bool	vertical_flip() const { return _vertical_flip; }
	bool	horizontal_flip() const { return _horizontal_flip; }
	void	vertical_flip(bool v) { _vertical_flip = v; }
	void	horizontal_flip(bool h) { _horizontal_flip = h; }
	int	scale() const { return _scale; }
	void	scale(int s) { _scale = s; }
	double	xshift() const { return _xshift; }
	void	xshift(double x) { _xshift = x; }
	double	yshift() const { return _yshift; }
	void	yshift(double y) { _yshift = y; }
	ImageTransformationStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
	virtual std::string	verboseinfo() const;
};

/**
 * \brief Step that performs the stacking operation
 */
class StackingStep : public ImageStep {
	ProcessingStepPtr	_baseimage;
	int	_numberofstars;
	int	_patchsize;
	double	_residual;
	int	_searchradius;
	bool	_notransform;
	bool	_usetriangles;
	bool	_rigid;
	bool	_rescale;
	void	rescale_image(ImagePtr image, double s);
public:
	StackingStep(NodePaths& parent);
	ProcessingStepPtr	baseimage() const { return _baseimage; }
	void	baseimage(ProcessingStepPtr b) { _baseimage = b; }
	int	numberofstars() const { return _numberofstars; }
	void	numberofstars(int n) { _numberofstars = n; }
	int	patchsize() const { return _patchsize; }
	void	patchsize(int p) { _patchsize = p; }
	double	residual() const { return _residual; }
	void	residual(double r) { _residual = r; }
	int	searchradius() const { return _searchradius; }
	void	searchradius(int s) { _searchradius = s; }
	bool	notransform() const { return _notransform; }
	void	notransform(bool n) { _notransform = n; }
	bool	usetriangles() const { return _usetriangles; }
	void	usetriangles(bool u) { _usetriangles = u; }
	bool	rigid() const { return _rigid; }
	void	rigid(bool r) { _rigid = r; }
	bool	rescale() const { return _rescale; }
	void	rescale(bool r) { _rescale = r; }
private:
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief 
 */
class SumStep : public ImageStep {
public:
	SumStep(NodePaths& parent) : ImageStep(parent) { }
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Processing step that combines the precursor images into a single image
 */
class LayerImageStep : public ImageStep {
public:
	LayerImageStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Step to combine images into an RGB image
 */
class RGBStep : public ImageStep {
public:
	RGBStep(NodePaths& parent) : ImageStep(parent) { }
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Step to combine RGB colors with luminance
 */
class LRGBStep : public ImageStep {
public:
	LRGBStep(NodePaths& parent) : ImageStep(parent) { }
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Image Step to change the color balance (see tools/image/color)
 */
class ColorStep : public ImageStep, public astro::adapter::ColorTransformBase {
public:
	ColorStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
	virtual ImagePtr	image();
};

/**
 * \brief HDR transformation step (see tools/image/hdr)
 */
class HDRStep : public ImageStep, public astro::image::post::HDR {
	int	_maskid;
public:
	int	maskid() const { return _maskid; }
	void	maskid(int m) { _maskid = m; }
public:
	HDRStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Rescaling transformation step (see tools/image/rescale)
 */
class RescaleStep : public ImageStep, public astro::image::post::Rescale {
public:
	RescaleStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
	virtual ImagePtr	image();
};

/**
 * \brief Color clampoing step
 */
class ColorclampStep : public ImageStep {
	double	_minimum;
public:
	double	minimum() const { return _minimum; }
	void	minimum(double m) { _minimum = m; }
private:
	double	_maximum;
public:
	double	maximum() const { return _maximum; }
	void	maximum(double m) { _maximum = m; }
public:
	ColorclampStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
	virtual ImagePtr	image();
};

/**
 * \brief Destarring step
 */
class DestarStep : public ImageStep {
	double	_radius;
public:
	double	radius() const { return _radius; }
	void	radius(double r) { _radius = r; }
public:
	DestarStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Step to stretch the luminance using a suitable stretching function
 */
class LuminanceStretchingStep : public ImageStep {
	adapter::LuminanceFactorPtr	_factor;
public:
	adapter::LuminanceFactorPtr	factor() const { return _factor; }
	void	factor(adapter::LuminanceFactorPtr f) { _factor = f; }
public:
	LuminanceStretchingStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Step to gamma correct an image
 */
class GammaStep : public ImageStep, public adapter::GammaTransformBase {
public:
	GammaStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
	virtual ImagePtr	image();
};

/**
 * \brief Step
 */
class LuminanceMappingStep : public ImageStep {
	adapter::LuminanceFunctionPtr	_luminancefunctionptr;
public:
	adapter::LuminanceFunctionPtr	luminancefunctionptr() const;
	void	luminancefunctionptr(adapter::LuminanceFunctionPtr l);
	LuminanceMappingStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
	virtual ImagePtr	image();
};

/**
 * \brief Deconvolution step
 */
class DeconvolutionStep : public ImageStep {
	ProcessingStepPtr	_psf;
	std::string	_method;
	int	_iterations;
	double	_epsilon;
	double	_K;
	double	_stddev;

	ProcessingStep::state	do_fourier(ImagePtr psf, ImagePtr img);
	ProcessingStep::state	do_pseudo(ImagePtr psf, ImagePtr img);
	ProcessingStep::state	do_wiener(ImagePtr psf, ImagePtr img);
	ProcessingStep::state	do_vancittert(ImagePtr psf, ImagePtr img);
	ProcessingStep::state	do_fastvancittert(ImagePtr psf, ImagePtr img);
	ProcessingStep::state	do_gold(ImagePtr psf, ImagePtr img);
public:
	ProcessingStepPtr	psf() const { return _psf; }
	void	psf(ProcessingStepPtr p) { _psf = p; }

	const std::string&	method() const { return _method; }
	void	method(const std::string& m) { _method = m; }

	int	iterations() const { return _iterations; }
	void	iterations(int i) { _iterations = i; }

	double	epsilon() const { return _epsilon; }
	void	epsilon(double e) { _epsilon = e; }

	double	K() const { return _K; }
	void	K(double k) { _K = k; }

	double	stddev() const { return _stddev; }
	void	stddev(double s) { _stddev = s; }

	DeconvolutionStep(NodePaths& parent);
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
* \brief Network Class to manage a complete network of interdependen steps
*/
class ProcessorNetwork : public NodePaths {
	typedef std::map<int, ProcessingStepPtr>	stepmap_t;
	typedef std::map<int, std::string>		id2namemap_t;
	typedef std::multimap<std::string, int>		name2idmap_t;

	stepmap_t	_steps;
	id2namemap_t	_id2names;
	name2idmap_t	_name2ids;
public:
	ProcessorNetwork();
	virtual ~ProcessorNetwork() { }
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
	std::vector<ProcessingThreadPtr>	_threads;
public:
	bool	hasneedswork();
	void	process();
	int	process(int id);
	int	process(const ProcessingStep::steps& steps);
	void	dump(std::ostream& out) const;
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
