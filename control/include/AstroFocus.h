/*
 * AstroFocus.h -- Focusing class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFocus_h
#define _AstroFocus_h

#include <AstroCamera.h>

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
	double	operator()(const ImagePtr image) = 0;
};
typedef std::shared_ptr<FocusEvaluator>	FocusEvaluatorPtr;

/**
 * \brief Class encapsulating the automatic focusing process
 *
 * In automatic focusing, the focus position is changed several times,
 * and an image is taken in these focus positions. The image is valuated
 * according to some focus figure of merit. This figure of merit is then
 * used to compute the best focus position, which is then set.
 */
class Focusing {
public:
	typedef enum { ONE_SIDED, TWO_SIDED } focus_mode;
	typedef enum { IDLE, MOVING, MEASURING, FOCUSED } focus_status;
private:
	astro::camera::CameraPtr	_camera;
public:
	astro::camera::CameraPtr	camera() { return _camera; }
private:
	astro::camera::FocuserPtr	_focuser;
public:
	astro::camera::FocuserPtr	focuser() { return _focuser; }
private:
	focus_mode	_mode;
	volatile focus_status	_status;
public:
	Focusing(astro::camera::CameraPtr camera,
		astro::camera::FocuserPtr focuser);
	void	start();
	focus_status	status() const { return _status; }
	void	cancel();
	focus_mode	mode() const { return _mode; }
	void	mode(focus_mode mode) { _mode = mode; }
};

} // namespace focusing
} // namespace astro

#endif /* _AstroFocus_h */
