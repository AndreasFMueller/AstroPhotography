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
};

/**
 * \brief Input for a focusing process
 *
 * This data object contains all the information needed for processing
 * the image to a focus position
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
 */
class FocusInputImages
	: public FocusInputBase, public std::map<unsigned long, ImagePtr> {
public:
	FocusInputImages(const FocusInput&);
	std::string	toString() const;
};

class FocusableImageConverter;
typedef std::shared_ptr<FocusableImageConverter> FocusableImageConverterPtr;
typedef std::shared_ptr<Image<float> >	FocusableImage;

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
	bool		_keep_images;
	FocusOutputPtr	_output;
public:
	bool	keep_images() const { return _keep_images; }
	void	keep_images(bool k) { _keep_images = k; }
	FocusOutputPtr	output() const { return _output; }
	FocusProcessor(const FocusInputBase&);
	void	process(const FocusElement&);
	void	process(const FocusInput& input);
	void	process(const FocusInputImages& input);
};

/**
 * \brief Extracting images suitable for focusing
 *
 * The camera may produce images that are not really suitable for
 * focusing. Bayer-Images e.g. have mixed color pixels that can
 * interfere with properly judging the focus quality. Images may also
 * have different pixel types, so this class serves to extract the version
 * of an image most suitable for focusing.
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
	typedef enum {
		BrennerHorizontal, BrennerVertical, BrennerOmni, FWHM, MEASURE
	} FocusEvaluatorType;
static FocusEvaluatorPtr	get(FocusEvaluatorType type);
static FocusEvaluatorPtr	get(FocusEvaluatorType type,
					const ImageRectangle& roi);
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

class CentroidSolver : public FocusSolver {
public:
	CentroidSolver();
	virtual ~CentroidSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class ParabolicSolver : public FocusSolver {
public:
	ParabolicSolver();
	virtual ~ParabolicSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class AbsoluteValueSolver : public ParabolicSolver {
public:
	AbsoluteValueSolver();
	virtual ~AbsoluteValueSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class MaximumSolver : public FocusSolver {
protected:
	float	maximum;
	float	minimum;
	int	maximumposition;
public:
	MaximumSolver();
	virtual ~MaximumSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class MinimumSolver : public FocusSolver {
public:
	MinimumSolver();
	virtual ~MinimumSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class BrennerSolver : public MaximumSolver {
public:
	BrennerSolver();
	virtual ~BrennerSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

/**
 * \brief Factory to produce solver classes
 */
class FocusSolverFactory {
public:
static std::list<std::string>	solvernames();
static FocusSolverPtr	get(const std::string& solver);
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
	typedef enum { IDLE, MOVING, MEASURING, FOCUSED, FAILED } state_type;
static std::string	state2string(state_type);
static state_type	string2state(const std::string& s);
private:
	volatile state_type	_status;
	void	status(state_type s) { _status = s; }
public:
	state_type	status() const { return _status; }

	// method for focusing
	typedef enum { BRENNER, FWHM, MEASURE } method_type;
static std::string	method2string(method_type);
static method_type	string2method(const std::string& name);
private:
	method_type	_method;
public:
	method_type	method() const { return _method; }
	void	method(method_type m) { _method = m; }

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
		return (_status == FOCUSED) || (_status == FAILED);
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
		int position, double value)
		: ImageCallbackData(image), _position(position), _value(value) {
	}
};

/**
 * \brief Callback data object to inform about a state change
 */
class FocusCallbackState : public astro::callback::CallbackData {
	Focusing::state_type	_state;
public:
	Focusing::state_type	state() const { return _state; }
	FocusCallbackState(Focusing::state_type state) : _state(state) { }
};

} // namespace focusing
} // namespace astro

#endif /* _AstroFocus_h */
