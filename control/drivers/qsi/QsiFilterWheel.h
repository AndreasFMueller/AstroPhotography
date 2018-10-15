/*
 * QsiFilterWheel.h -- QSI cameras with filter wheels
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _QsiFilterWheel_h
#define _QsiFilterWheel_h

#include <AstroCamera.h>
#include <QsiCamera.h>
#include <vector>
#include <atomic>
#include <thread>

namespace astro {
namespace camera {
namespace qsi {

/**
 * \brief FilterWheel interface class
 */
class QsiFilterWheel : public FilterWheel {
	QsiCamera&	_camera;
	unsigned int	nfilters;
	std::vector<std::string>	filternames;

	// state variables, need to be protected from concurrent access
	std::atomic_uint	lastPosition;
	std::atomic<FilterWheel::State>	lastState;

	// auxiliary variables for moving the filter wheel
	std::thread	*_thread;
	volatile std::atomic_bool	_movement_done;
public:
	QsiFilterWheel(QsiCamera& camera);
	virtual ~QsiFilterWheel();

	virtual unsigned int	nFilters();
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual void	select(const std::string& filtername);
	virtual std::string	filterName(size_t filterindex);
	virtual FilterWheel::State	getState();
	virtual std::string	userFriendlyName() const {
		return _camera.userFriendlyName();
	}

	// methods associated with moving the filterwheel, this must
	// be public because the main function of the thread must be
	// able to access it.
	void	move(size_t newposition);
};

} // namespace qsi
} // namespace camera
} // namespace astro

#endif /* _QsiFilterWheel_h */
