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
 * \brief Callback data for the focusing process
 */
class FocusCallbackData : public astro::callback::ImageCallbackData {
	double	_value;
public:
	double	value() const { return _value; }
	FocusCallbackData(astro::image::ImagePtr image, double value)
		: ImageCallbackData(image), _value(value) { }
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
	typedef enum { IDLE, MOVING, MEASURING, FOCUSED, FAILED } focus_status;
static std::string	name_of_status(focus_status);
private:
	volatile focus_status	_status;
	void	status(focus_status s) { _status = s; }
public:
	focus_status	status() const { return _status; }

	// method for focusing
	typedef enum { FWHM, MEASURE } focus_method;
static std::string	name_of_method(focus_method);
static focus_method	method_from_name(const std::string& name);
private:
	focus_method	_method;
public:
	focus_method	method() const { return _method; }
	void	method(focus_method m) { _method = m; }

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

} // namespace focusing
} // namespace astro

#endif /* _AstroFocus_h */
