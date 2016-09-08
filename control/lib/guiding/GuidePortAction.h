/*
 * GuidePortAction.h -- asynchronous action for the guider port
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _GuidePortAction_h
#define _GuidePortAction_h

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
class GuidePortAction : public Action {
	GuidePortPtr	_guideport;
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
	
	GuidePortAction(GuidePortPtr guideport, const Point& correction,
		double deltat)
		: _guideport(guideport), _correction(correction),
		  _deltat(deltat) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "GuidePortAction %s",
			correction.toString().c_str());
		_sequential = false;
		_stepping = false;
	}
	void	execute();
};

} // namespace guiding
} // namespace astro

#endif /* _GuidePortAction_h */
