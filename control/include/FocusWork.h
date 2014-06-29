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
	FocusEvaluatorPtr	_evaluator;
public:
	FocusEvaluatorPtr	evaluator() { return _evaluator; }
	void	evaluator(FocusEvaluatorPtr e) { _evaluator = e; }
private:
	astro::callback::CallbackPtr	_callback;
public:	
	astro::callback::CallbackPtr	callback() { return _callback; }
	void	callback(astro::callback::CallbackPtr c) { _callback = c; }
private:
	bool	complete();
	Focusing&	_focusing;
public:
	FocusWork(Focusing& focusing);
	void	main(astro::thread::Thread<FocusWork>& thread);

	// forbid copying by declaring copy constructor and assignment
	// operator private
private:
	FocusWork(const FocusWork& other);
	FocusWork&	operator=(const FocusWork& other);
};

} // namespace focusing
} // namespace astro

#endif /* _FocusWork_h */
