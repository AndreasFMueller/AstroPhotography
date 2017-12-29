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
#include <thread>
#include <mutex>

namespace astro {
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
 * \brief ProcessingStep base class
 *
 * All image processing steps are of this type. Each processing step has an
 * interface to access a preview. If the step takes time, and cannot be
 * completed pixel by pixel, then the work method will take some time
 * to complete, it is probably a good idea to run it in a separate thread.
 * 
 */
class ProcessingStep {
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
	virtual ~ProcessingStep();
private:	// prevent copying
	ProcessingStep(const ProcessingStep& other);
	ProcessingStep&	operator=(const ProcessingStep& other);
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

class ProcessingThread : public std::thread {
	ProcessingStepPtr	_step;
public:
	ProcessingStepPtr	step() const { return _step; }
	ProcessingThread(ProcessingStepPtr step);
	~ProcessingThread();
	void	work();
};
typedef std::shared_ptr<ProcessingThread>	ProcessingThreadPtr;


class ImageStep : public ProcessingStep {
protected:
	ImagePtr	_image;
public:
	virtual ImagePtr	image() { return _image; }
	ImageStep() : ProcessingStep() { }
	ImageSequence	precursorimages(std::vector<int> exlude = std::vector<int>());
	virtual ProcessingStep::state	do_work() = 0;
	ImagePtr	precursorimage(std::vector<int> exlude = std::vector<int>());
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
protected:
	bool	_exists;
	bool	exists();
public:
	FileImageStep(const std::string& filename);
	~FileImageStep();
	virtual time_t	when() const;
	virtual ProcessingStep::state	status();
	virtual ImagePtr image();
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Writeable File image step
 *
 * The writeable file image step writes an image to a file when its
 * dependencies have changed
 */
class WriteableFileImageStep : public FileImageStep {
	std::recursive_mutex	_mutex;
public:
	WriteableFileImageStep(const std::string& filename);
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

/** 
 * \brief Flat Image step
 *
 * The flat image step constructs a flat image from a set of precursor
 * images.
 */
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
	virtual ProcessingStep::state	status();
	virtual std::string	what() const;
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
public:
	StackingStep();
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
private:
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};


/**
 * \brief Image Step to change the color balance (see tools/image/color)
 */
class ColorStep : public ImageStep, public astro::adapter::ColorTransformBase {
public:
	ColorStep();
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
	HDRStep();
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

/**
 * \brief Rescaling transformation step (see tools/image/rescale)
 */
class RescaleStep : public ImageStep, public astro::image::post::Rescale {
public:
	RescaleStep();
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
	ColorclampStep();
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
	DestarStep();
	virtual ProcessingStep::state	do_work();
	virtual std::string	what() const;
};

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
	std::vector<ProcessingThreadPtr>	_threads;
public:
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
