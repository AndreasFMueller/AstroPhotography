/*
 * SxFilterWheel.h -- Filter wheel implementation for the Filter wheel
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _SxFilterWheel_h
#define _SxFilterWheel_h

#include <AstroCamera.h>
#include <AstroUSB.h>
#include <hidapi.h>
#include <thread>
#include <AstroUtils.h>

namespace astro {
namespace camera {
namespace sx {

/**
 * \brief Starlight Express filterwheel class
 */
class SxFilterWheel : public FilterWheel {
	unsigned int	nfilters;
	std::vector<std::string>	filternames;
	hid_device	*_hid;
	astro::thread::Barrier		_barrier;
	std::thread	_thread;
	bool	_terminate;
	std::recursive_mutex	_mutex;
	std::condition_variable_any	_condition;
	void	stop();
	static void	main(SxFilterWheel *filterwheel) noexcept;
	void	run();
private:

	typedef enum filterwheel_cmd_e {
		no_command = 0,
		select_filter = 1,
		current_filter = 2,
		get_total = 3
	} filterwheel_cmd_t;
	filterwheel_cmd_t	pending_cmd;

	void	send_command(filterwheel_cmd_t cmd, int arg = 0);
	int	read_response();
	int	try_complete();

	typedef enum filterwheel_state_e { 
		unknown = 0,
		moving = 1,
		idle = 2
	} filterwheel_state_t;
	filterwheel_state_t	state;

	// the current position is the number of the filter as
	// defined by the SX filterwheel (i.e. 1-nFilters), not
	// the filterindex used by the FilterWheel class
	int	currentposition;
public:
	SxFilterWheel(const DeviceName& name);
	~SxFilterWheel();
protected:
	virtual unsigned int	nFilters0();
public:
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual void	select(const std::string& filtername);
	virtual std::string	filterName(size_t filterindex);
	virtual FilterWheel::State	getState();
	virtual std::string	userFriendlyName() const;
};

} // namespace sx
} // namespace camera
} // namespace astro

#endif /* _SxFilterWheel_h */
