/*
 * AstroFocus.h -- Focusing class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFocus_h
#define _AstroFocus_h

#include <AstroCamera.h>
#include <AstroCallback.h>
#include <Thread.h>

namespace astro {
namespace focusing {

class FocusableImageConverter;
typedef std::shared_ptr<FocusableImageConverter> FocusableImageConverterPtr;
typedef std::shared_ptr<Image<float> >	FocusableImage;

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
public:
	virtual double	operator()(const ImagePtr image) = 0;
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
	typedef enum { BrennerHorizontal, BrennerVertical, BrennerOmni } FocusEvaluatorType;
static FocusEvaluatorPtr	get(FocusEvaluatorType type);
static FocusEvaluatorPtr	get(FocusEvaluatorType type,
					const ImageRectangle& roi);
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
	typedef enum { FWHM, MEASURE } method_type;
static std::string	method2string(method_type);
static method_type	string2method(const std::string& name);
private:
	method_type	_method;
public:
	method_type	method() const { return _method; }
	void	method(method_type m) { _method = m; }

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
	~Focusing();
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
