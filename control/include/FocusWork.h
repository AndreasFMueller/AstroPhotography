/*
 * FocusWork.h -- thread performing the focusing
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusWork_h
#define _FocusWork_h

#include <AstroUtils.h>
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
	unsigned long	_min;
public:
	unsigned long	min() const { return _min; }
	void	min(unsigned long m) { _min = m; }
private:
	unsigned long	_max;
public:
	unsigned long	max() const { return _max; }
	void	max(unsigned long m) { _max = m; }
	unsigned long	steps() const { return _focusing.steps(); }
	unsigned long	backlash();
	void	moveto(unsigned long position);
	astro::camera::CcdPtr	ccd() {
		return _focusing.ccd();
	}
	astro::camera::FocuserPtr	focuser() {
		return _focusing.focuser();
	}
	astro::camera::Exposure	exposure() {
		return _focusing.exposure();
	}
	FocusEvaluatorPtr	evaluator() const {
		return _focusing.evaluator();
	}
	FocusSolverPtr	solver() const {
		return _focusing.solver();
	}
	astro::callback::CallbackPtr	callback() {
		return _focusing.callback();
	}
	void	callback(ImagePtr image, int position, double value);
	void	callback(Focusing::state_type state);
protected:
	bool	complete();
	Focusing&	_focusing;
	Focusing::state_type	focusingstatus();
	void	focusingstatus(Focusing::state_type s);
public:
	FocusWork(Focusing& focusing);
	virtual ~FocusWork() { }
	virtual void	main(astro::thread::Thread<FocusWork>& thread);

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
	unsigned long	position;
	double	value;
	FocusValue(unsigned long _position, double _value)
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
	unsigned long	length() const;
	unsigned long	center() const;
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
	FocusValue	measureat(unsigned long pos);
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
