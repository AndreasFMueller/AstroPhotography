/*
 * GuiderPortAction.h -- asynchronous action for the guider port
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuiderPortAction_h
#define _GuiderPortAction_h

#include <AstroGuiding.h>
#include <AstroUtils.h>
#include <AstroCamera.h>

using namespace astro::callback;
using namespace astro::camera;

namespace astro {
namespace guiding {

/**
 * \brief action class for asynchronous guider port actions
 */
class GuiderPortAction : public Action {
	GuiderPortPtr	_guiderport;
	Point	_correction;
	double	_deltat;
	bool	_sequential;
public:
	bool	sequential() const { return _sequential; }
	void	sequential(bool s) { _sequential = s; }
private:
	bool	_stepping;
public:
	bool	stepping() const { return _stepping; }
	void	stepping(bool s) { _stepping = s; }
	
	GuiderPortAction(GuiderPortPtr guiderport, const Point& correction,
		double deltat)
		: _guiderport(guiderport), _correction(correction),
		  _deltat(deltat) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "GuiderPortAction %s",
			correction.toString().c_str());
		_sequential = false;
		_stepping = false;
	}
	void	execute();
};

} // namespace guiding
} // namespace astro

#endif /* _GuiderPortAction_h */
