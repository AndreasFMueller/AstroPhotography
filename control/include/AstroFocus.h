/*
 * AstroFocus.h -- Focusing class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroFocus_h
#define _AstroFocus_h

namespace astro {
namespace focusing {

class Focusing {
public:
	typedef enum { ONE_SIDED, TWO_SIDED } focus_mode;
	typedef enum { IDLE, MOVING, MEASURING, FOCUSED } focus_status;
private:
	CameraPtr	_camera;
	FocuserPtr	_focuser;
	focus_mode	_mode;
	volatile focus_status	_status;
public:
	Focusing(CameraPtr camera, FocuserPtr focuser);
	void	start();
	focus_status	status() const { return status; }
	void	cancel();
	focus_mode	mode() const { return _mode; }
	void	mode(focus_mode mode) { _mode = mode; }
};

} // namespace focusing
} // namespace astro

#endif /* _AstroFocus_h */
