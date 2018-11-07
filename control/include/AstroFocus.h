/*
 * AstroFocus.h -- Focusing class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFocus_h
#define _AstroFocus_h

#include <AstroCamera.h>
#include <AstroCallback.h>
#include <AstroUtils.h>
#include <queue>

namespace astro {
namespace focusing {

/**
 * \brief Base class for focus input
 *
 * This method contains just parameters needed to evaluate all the images 
 * of a sequence to find the focus
 */
class FocusInputBase {
	image::ImageRectangle	_rectangle;
	std::string		_method;
	std::string		_solver;
protected:
	ImagePtr	image(const std::string& filename) const;
public:
	const ImageRectangle&	rectangle() const { return _rectangle; }
	void	rectangle(const ImageRectangle& r) { _rectangle = r; }

	const std::string	method() const { return _method; }
	void	method(const std::string& m) { _method = m; }

	const std::string	solver() const { return _solver; }
	void	solver(const std::string& s) { _solver = s; }

	FocusInputBase();
	FocusInputBase(const std::string& method, const std::string& solver);
};

/**
 * \brief Input for a focusing process
 *
 * This data object contains all the information needed for processing
 * the image to a focus position, including an image file name for each
 * focus position.
 */
class FocusInput
	: public FocusInputBase, public std::map<unsigned long, std::string> {
public:
	FocusInput();
	std::string     toString() const;
	ImagePtr	image(unsigned long) const;
};

/**
 * \brief A class to hold focus input images
 *
 * This object contains actual input images for the focus evaluation
 */
class FocusInputImages
	: public FocusInputBase, public std::map<unsigned long, ImagePtr> {
public:
	FocusInputImages(const FocusInput&);
	std::string	toString() const;
};

// focusable images are always float images, this allows us to simplify
// the algorithms somewhat, because we know that they are float images
typedef std::shared_ptr<Image<float> >	FocusableImage;

class FocusImagePreconditioner : public ConstImageAdapter<float> {
	const ConstImageAdapter<float>&	_image;
	FocusableImage	_imageptr;
	float	_noisefloor;
	float	_mean;
	float	_max;
	float	_top;
	float	_stddev;
public:
	float	noisefloor() const { return _noisefloor; }
	float	mean() const { return _mean; }
	float	max() const { return _max; }
	float	top() const { return _top; }
	void	top(float t) { _top = t; }
	float	stddev() const { return _stddev; }
	FocusImagePreconditioner(FocusableImage image);
	virtual float	pixel(int x, int y) const;
};

// The FocusableImageConverter is used to convert the input image into
// a focusable image. In this step we also extract the rectangle.
class FocusableImageConverter;
typedef std::shared_ptr<FocusableImageConverter> FocusableImageConverterPtr;

/**
 * \brief The FocusElement collects all the data that is accumualted in focusing
 */
class FocusElement {
	unsigned long	_pos;
public:
	FocusElement(unsigned long pos);
	unsigned long	pos() const { return _pos; }
	std::string	filename;
	ImagePtr	raw_image;
	std::string	method;
	ImagePtr	processed_image;
	double		value;
	ImagePtr	image() const;
	std::string	toString() const;
};
typedef std::shared_ptr<FocusElement>	FocusElementPtr;

/**
 * \brief A queue used to forward focus elements between threads
 */
class FocusElementQueue : std::queue<FocusElementPtr> {
	std::mutex	_mutex;
	std::condition_variable	_condition;
	bool	_terminated;
public:
	bool	terminated() const { return _terminated; }
	FocusElementQueue();
	void	put(FocusElementPtr feptr);
	void	terminate();
	FocusElementPtr	get();
};
typedef std::shared_ptr<FocusElementQueue>	FocusElementQueuePtr;

/**
 * \brief Callback data to report focus computation progress
 */
class FocusElementCallbackData : public callback::CallbackData {
	unsigned long		_position;
	astro::image::ImagePtr	_raw_image;
	std::string		_method;
	astro::image::ImagePtr	_processed_image;
	double			_value;
public:
	unsigned long	position() const { return _position; }
	astro::image::ImagePtr	raw_image() const { return _raw_image; }
	std::string	method() const { return _method; }
	astro::image::ImagePtr	processed_image() const { return _processed_image; }
	double	value() const { return _value; }

	FocusElementCallbackData(const FocusElement& e);

	std::string	toString() const;
};

/**
 * \brief Base class for focs callbacks
 *
 * This callback simplifies sending callback information because it
 * takes a FocusElement, converts it into FocusCallbackData and
 * sends this through the usual channel.
 */
class FocusElementCallback : public callback::Callback {
protected:
	FocusElementCallbackData	*unpacked(callback::CallbackDataPtr cbd);
public:
	FocusElementCallback();
	virtual ~FocusElementCallback();
	virtual void	handle(FocusElementCallbackData& fe) const = 0;
	virtual callback::CallbackDataPtr operator()(callback::CallbackDataPtr cbd);
	void	send(const FocusElement& element);
};

/**
 * \brief Container class that contains focus position and value
 *
 * Any focusing algorithm does this by first measuring the focus
 * measure for a couple of focus positions and then finds the best
 * focus position.
 */
class FocusItem {
	int	_position;
	float	_value;
public:
	int	position() const { return _position; }
	float	value() const { return _value; }
	FocusItem(int position, float value)
		: _position(position), _value(value) { }
	bool	operator<(const FocusItem& other) const {
		return _position < other.position();
	}
	bool	operator==(const FocusItem& other) const {
		return _position == other.position();
	}
	bool	operator!=(const FocusItem& other) const {
		return _position != other.position();
	}
};

typedef std::set<FocusItem>	FocusItems;

/**
 * \brief Output of the Focus Processor
 */
class FocusOutput : public FocusInputBase,
		public std::map<unsigned long, FocusElement> {
public:
	FocusOutput(const std::string& method, const std::string& solver);
	FocusOutput(const FocusInputBase&);
	FocusOutput(const FocusInput&);
	FocusOutput(const FocusInputImages&);
	FocusItems	items() const;
};
typedef std::shared_ptr<FocusOutput>	FocusOutputPtr;

/**
 * \brief Processor class that takes the FocusInput and produces a solution
 */
class FocusProcessor {
	bool			_keep_images;
	FocusOutputPtr		_output;
	image::ImageRectangle	_rectangle;
public:
	bool	keep_images() const { return _keep_images; }
	void	keep_images(bool k) { _keep_images = k; }
	const image::ImageRectangle&	rectangle() const { return _rectangle; }
	void	rectangle(const image::ImageRectangle& r) { _rectangle = r; }
	FocusOutputPtr	output() const { return _output; }
	FocusProcessor(const FocusInputBase&);
	FocusProcessor(const std::string& method, const std::string& solver);
	void	process(FocusElement&);
	void	process(FocusInput& input);
	void	process(FocusInputImages& input);
};

/**
 * \brief Extracting images suitable for focusing
 *
 * The camera may produce images that are not really suitable for
 * focusing. Bayer-Images e.g. have mixed color pixels that can
 * interfere with properly judging the focus quality. Images may also
 * have different pixel types, so this class serves to extract the version
 * of an image most suitable for focusing.
 *
 * This class is also a factory class that produces instances of the
 * converter class.
 */
class FocusableImageConverter {
public:
	static FocusableImageConverterPtr	get();	
	static FocusableImageConverterPtr	get(const ImageRectangle& rectangle);	
	virtual FocusableImage	operator()(ImagePtr image) = 0;
};

Image<unsigned char>	*UnsignedCharImage(ImagePtr image);

/**
 * \brief FocusEvaluator class to evaluate the focus of an image
 *
 * Base class defining the interface. Derived classes are expected to
 * implement the operator() which returns the focus figure of merit for
 * an image. This figure of merit must be smallest when focus is achieved,
 * and should be roughly proportional to the offset from the correct
 * the focus position.
 */
class FocusEvaluator {
protected:
	ImagePtr	_evaluated_image;
public:
	virtual ~FocusEvaluator() { }
	virtual double	operator()(const ImagePtr image) = 0;
	ImagePtr	evaluated_image() const { return _evaluated_image; }
};
typedef std::shared_ptr<FocusEvaluator>	FocusEvaluatorPtr;

/**
 * \brief Factory class to build FocusEvaluators
 *
 * Most focus evaluators implemented in the library have a region of
 * interest defined.
 */
class FocusEvaluatorFactory {
public:
static FocusEvaluatorPtr	get(const std::string& type);
static FocusEvaluatorPtr	get(const std::string& type,
					const ImageRectangle& roi);
static std::list<std::string>	evaluatornames();
};

/**
 * \brief Solver class to compute the solution of the focusing problem
 *
 * This base class defines the interface for focus solver classes
 */
class FocusSolver {
public:
	virtual ~FocusSolver() { }
	virtual int	position(const FocusItems& focusitems) = 0;
};

typedef std::shared_ptr<FocusSolver>	FocusSolverPtr;

/**
 * \brief Factory to produce solver classes
 */
class FocusSolverFactory {
public:
static std::list<std::string>	solvernames();
static FocusSolverPtr	get(const std::string& solver);
};

/**
 * \brief Focus namespace for common stuff
 */
class Focus {
public:
	// focusing status (what is it doing right now?)
	typedef enum { IDLE, MOVING, MEASURING, MEASURED, FOCUSED, FAILED } state_type;
static std::string	state2string(state_type);
static state_type	string2state(const std::string& s);
};

/**
 * \brief Parameters for the focusing process
 *
 * Setting up the focusing process needs a lot of parameters which are
 * collected in this class. The construtors are designed in such a
 * way that a constructed instance of this class is always consistent.
 */
class FocusParameters {

	// focus position interval to scan
private:
	unsigned long	_minposition;
	unsigned long	_maxposition;
protected:
	void	minposition(unsigned long m) { _minposition = m; }
	void	maxposition(unsigned long m) { _maxposition = m; }
public:
	unsigned long	minposition() const { return _minposition; }
	unsigned long	maxposition() const { return _maxposition; }

	// subdivision steps for the interval
private:
	int	_steps;
public:
	int	steps() const { return _steps; }
	void	steps(int s);

	// the exposure
private:
	camera::Exposure	_exposure;
public:
	const camera::Exposure	exposure() const { return _exposure; }
	void	exposure(const camera::Exposure& e);

private:
	std::string	_method;
	std::string	_solver;
public:
	const std::string&	method() const { return _method; }
	const std::string&	solver() const { return _solver; }
	void	method(const std::string& m);
	void	solver(const std::string& s);

	FocusParameters(unsigned long minposition, unsigned long maxposition);
	FocusParameters(const FocusParameters& parameters);
};


/**
 * \brief The base class for focusing processes
 *
 * This class implements the general logic of the focusing process, without
 * the nitty gritt ydetails of how to moving the focus position and getting
 * images
 */
class FocusProcessBase : public FocusParameters {
	// the main methods needed to implement the focusing process
public:
	virtual void		moveto(unsigned long position) = 0;
	virtual ImagePtr	get() = 0;
private:
	thread::Waiter<Focus::state_type>	_status;
	void	status(Focus::state_type s) { _status = s; }
public:
	Focus::state_type	status() const { return _status; }

	// callback
private:
	callback::CallbackPtr	_callback;
public:
	callback::CallbackPtr	callback() const { return _callback; }
	void	callback(callback::CallbackPtr c) { _callback = c; }

	// constructors
	FocusProcessBase(unsigned long minposition, unsigned long maxposition);
	FocusProcessBase(const FocusParameters& parameters);
	virtual ~FocusProcessBase();

private:
	void	reportState();
	void	reportImage(image::ImagePtr);
	void	reportFocusElement(const FocusElement&);
	void	replaceUuid(image::ImageBase&);
private:
	std::atomic_bool	_running;
	std::thread	_measure_thread;
	std::thread	_evaluate_thread;
public:
	bool	completed() const;
	// start the process
	void	start();
	void	stop();
	void	wait();
	// thread main functions
	void	measure();
	void	evaluate();
private:
	FocusElementQueuePtr	_focus_elements;
	bool	measure0();
	bool	evaluate0();
};

/**
 * \brief Focus Process using a CCD and a focuser directly
 */
class FocusProcess : public FocusProcessBase {
	camera::CcdPtr		_ccd;
	camera::FocuserPtr	_focuser;
public:
	camera::CcdPtr		ccd() const { return _ccd; }
	camera::FocuserPtr	focuser() const { return _focuser; }
	
	FocusProcess(const FocusParameters& parameters,
		camera::CcdPtr ccd, camera::FocuserPtr focuser);
	FocusProcess(unsigned long minposition, unsigned long maxposition,
		camera::CcdPtr ccd, camera::FocuserPtr focuser);
	virtual void	moveto(unsigned long);
	virtual ImagePtr	get();
};

/**
 * \brief Class encapsulating the automatic focusing process
 *
 * In automatic focusing, the focus position is changed several times,
 * and an image is taken in these focus positions. The image is valuated
 * according to some focus figure of merit. This figure of merit is then
 * used to compute the best focus position, which is then set.
 */
class Focusing : public FocusProcess {
	Focusing(const Focusing& other) = delete;
	Focusing&	operator=(const Focusing& other) = delete;
public:
	Focusing(astro::camera::CcdPtr ccd,
		astro::camera::FocuserPtr focuser);
	virtual ~Focusing();
	void	start(int min, int max);
	void	cancel();
};

typedef std::shared_ptr<Focusing>	FocusingPtr;

/**
 * \brief Callback data for the focusing process
 */
class FocusCallbackData : public astro::callback::ImageCallbackData {
	int	_position;
	double	_value;
public:
	int	position() const { return _position; }
	double	value() const { return _value; }
	FocusCallbackData(astro::image::ImagePtr image,
		int position, double value);
	FocusCallbackData(const FocusElement& fe);
};

/**
 * \brief Callback data object to inform about a state change
 */
class FocusCallbackState : public astro::callback::CallbackData {
	Focus::state_type	_state;
public:
	Focus::state_type	state() const { return _state; }
	FocusCallbackState(Focus::state_type state) : _state(state) { }
};

} // namespace focusing
} // namespace astro

#endif /* _AstroFocus_h */
