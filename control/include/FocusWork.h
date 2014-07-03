/*
 * FocusWork.h -- thread performing the focusing
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusWork_h
#define _FocusWork_h

#include <Thread.h>
#include <AstroCamera.h>
#include <AstroFocus.h>

namespace astro {
namespace focusing {

/**
 * \brief Focusing work class
 *
 * The focusing process controls the camera and the focuser. It takes
 * several images, estimates the focus and then moves to the optimal
 * focus position.
 */
class FocusWork {
	unsigned short	_min;
public:
	unsigned short	min() const { return _min; }
	void	min(unsigned short m) { _min = m; }
private:
	unsigned short	_max;
public:
	unsigned short	max() const { return _max; }
	void	max(unsigned short m) { _max = m; }
private:
	unsigned short	_steps;
public:
	unsigned short	steps() const { return _steps; }
	void	steps(unsigned short s);
private:
	astro::camera::CcdPtr	_ccd;
public:
	astro::camera::CcdPtr	ccd() { return _ccd; }
	void	ccd(astro::camera::CcdPtr c) { _ccd = c; }
private:
	astro::camera::FocuserPtr	_focuser;
public:
	astro::camera::FocuserPtr	focuser() { return _focuser; }
	void	focuser(astro::camera::FocuserPtr f) { _focuser = f; }
private:
	astro::camera::Exposure	_exposure;
public:
	const astro::camera::Exposure&	exposure() { return _exposure; }
	void	exposure(const astro::camera::Exposure& e) { _exposure = e; }
private:
	astro::callback::CallbackPtr	_callback;
public:	
	astro::callback::CallbackPtr	callback() { return _callback; }
	void	callback(astro::callback::CallbackPtr c) { _callback = c; }
	void	callback(ImagePtr image, double value);
protected:
	bool	complete();
	Focusing&	_focusing;
	Focusing::focus_status	focusingstatus() { return _focusing.status(); }
	void	focusingstatus(Focusing::focus_status s) { _focusing.status(s); }
public:
	FocusWork(Focusing& focusing);
	virtual ~FocusWork() { }
	virtual void	main(astro::thread::Thread<FocusWork>& thread) = 0;

	// forbid copying by declaring copy constructor and assignment
	// operator private
private:
	FocusWork(const FocusWork& other);
	FocusWork&	operator=(const FocusWork& other);
protected:
	Image<unsigned char>	*green(ImagePtr image);
};

/**
 * \brief Focusing work class method based on a V-Curve
 *
 * This work class moves the focuser to a list of focus positions and
 * determines the FWHM through an FWHM2 evaluator. From the various
 * FWHM measures obtained, it infers the optimal focus position.
 */
class VCurveFocusWork : public FocusWork {
	ImagePtr	combine(ImagePtr image, FWHMInfo& fwhminfo);
public:
	VCurveFocusWork(Focusing& focusing) : FocusWork(focusing) { }
	virtual ~VCurveFocusWork() { }
	virtual void	main(astro::thread::Thread<FocusWork>& thread);
};

/**
 * \brief Focus information
 */
class FocusValue {
public:
	unsigned short	position;
	double	value;
	FocusValue(unsigned short _position, double _value)
		: position(_position), value(_value) { }
	std::string	toString() const;
	bool	operator==(const FocusValue& other) const;
};

/**
 * \brief Interval of focus positions and values
 */
class FocusInterval : public std::pair<FocusValue, FocusValue> {
public:
	FocusInterval(const FocusValue& left, const FocusValue& right);
	unsigned short	length() const;
	unsigned short	center() const;
	FocusValue&	left() { return first; }
	FocusValue&	right() { return second; }
	const FocusValue&	left() const { return first; }
	const FocusValue&	right() const { return second; }
	std::string	toString() const;
	FocusInterval	operator-(const FocusInterval& other) const;
};

/**
 * \brief Focusing work class method based  on a focus measure
 *
 * This work class moves the focuser with the goal to maximize some focus
 * measure
 */
class MeasureFocusWork : public FocusWork {
	int	counter;
	FocusValue	measureat(unsigned short pos);
	FocusInterval	subdivide(const FocusInterval& interval);
	ImagePtr	combine(ImagePtr image, FocusInfo& focusinf);
public:
	MeasureFocusWork(Focusing& focusing) : FocusWork(focusing) { }
	virtual ~MeasureFocusWork() { }
	virtual void	main(astro::thread::Thread<FocusWork>& thread);
};

} // namespace focusing
} // namespace astro

#endif /* _FocusWork_h */
