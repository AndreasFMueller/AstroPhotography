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
	ImagePtr	processed_image;
	double		value;
	ImagePtr	image() const;
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
	typedef enum { IDLE, MOVING, MEASURING, FOCUSED, FAILED } state_type;
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
	void	callbacu(callback::CallbackPtr c) { _callback = c; }

	// constructors
	FocusProcessBase(unsigned long minposition, unsigned long maxposition);
	FocusProcessBase(const FocusParameters& parameters);
	virtual ~FocusProcessBase();

private:
	void	reportState();
	void	reportImage(image::ImagePtr);
	void	reportFocusElement(const FocusElement&);

private:
	std::atomic_bool	_running;
	std::thread	_thread;
public:
	// start the process
	void	start();
	void	stop();
	void	wait();
	void	run();
private:
	bool	run0();
};

/**
 * \brief Focus Process using a CCD and a focuser directly
 */
class FocusProcess : public FocusProcessBase {
	camera::CcdPtr		_ccd;
	camera::FocuserPtr	_focuser;
public:
	FocusProcess(const FocusParameters& parameters,
		camera::CcdPtr ccd, camera::FocuserPtr focuser);
	FocusProcess(unsigned long minposition, unsigned long maxposition,
		camera::CcdPtr ccd, camera::FocuserPtr focuser);
	virtual void	moveto(unsigned long);
	virtual ImagePtr	get();
};

// we need the FocusWork forward declaration in the next class
class FocusWork;

/**
 * \brief Class encapsulating the automatic focusing process
 *
 * In automatic focusing, the focus position is changed several times,
 * and an image is taken in these focus positions. The image is valuated
 * according to some focus figure of merit. This figure of merit is then
 * used to compute the best focus position, which is then set.
 */
class Focusing {
	// callback for images
	astro::callback::CallbackPtr	_callback;
public:
	astro::callback::CallbackPtr	callback() { return _callback; }
	void	callback(astro::callback::CallbackPtr c) { _callback = c; }

	// focusing status (what is it doing right now?)
private:
	volatile Focus::state_type	_status;
	void	status(Focus::state_type s) { _status = s; }
public:
	Focus::state_type	status() const { return _status; }

	// method for focusing
private:
	std::string	_method;
public:
	std::string	method() const { return _method; }
	void	method(const std::string& m) { _method = m; }

	// matching the method, we have an evaluator
private:
	FocusEvaluatorPtr	_evaluator;
public:
	FocusEvaluatorPtr	evaluator() const { return _evaluator; }
	void	evaluator(FocusEvaluatorPtr e) { _evaluator = e; }

	// matching focus solver that works with the evaluator
private:
	FocusSolverPtr	_solver;
public:
	FocusSolverPtr	solver() const { return _solver; }
	void	solver(FocusSolverPtr s) { _solver = s; }

	// CCD to be used to get images
private:
	astro::camera::CcdPtr	_ccd;
public:
	astro::camera::CcdPtr	ccd() { return _ccd; }

	// focuser to use to change focus
private:
	astro::camera::FocuserPtr	_focuser;
public:
	astro::camera::FocuserPtr	focuser() { return _focuser; }

	// subdivision steps for the interval
private:
	int	_steps;
public:
	int	steps() const { return _steps; }
	void	steps(int s) { _steps = s; }

	// exposure specification for images
private:
	astro::camera::Exposure	_exposure;
public:
	astro::camera::Exposure	exposure() { return _exposure; }
	void	exposure(astro::camera::Exposure e) { _exposure = e; }

	bool	completed() const {
		return (_status == Focus::FOCUSED) || (_status == Focus::FAILED);
	}
private:
	Focusing(const Focusing& other);
	Focusing&	operator=(const Focusing& other);
public:
	Focusing(astro::camera::CcdPtr ccd,
		astro::camera::FocuserPtr focuser);
	virtual ~Focusing();
	void	start(int min, int max);
	void	cancel();
public:
	astro::thread::ThreadPtr	thread;
	FocusWork	*work;
	friend class FocusWork;	// allow the FocusWork class update the status
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
