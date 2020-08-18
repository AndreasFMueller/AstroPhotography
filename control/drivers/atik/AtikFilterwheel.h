/*
 * AtikFilterwheel.h -- declaration of ATIK filterwheel class
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AtikFilterwheel_h
#define _AtikFilterwheel_h

#include <atikccdusb.h>
#include <AstroCamera.h>
#include <AtikCamera.h>

namespace astro {
namespace camera {
namespace atik {

class AtikFilterwheel : public FilterWheel {
	AtikCamera&	_camera;
	std::recursive_mutex		_mutex;
	std::condition_variable_any	_condition;
	std::thread			_thread;
	bool	_running;
	unsigned int	filtercount;
	bool		moving;
	unsigned int	current;
	unsigned int	target;
	void	query();
	static void	main(AtikFilterwheel *fw) noexcept;
	void	run();
	void	stop();
public:
	AtikFilterwheel(AtikCamera& camera);
protected:
	virtual unsigned int	nFilters0();
public:
	virtual unsigned int	currentPosition();
	virtual void	select(size_t filterindex);
	virtual State	getState();
	virtual std::string	userFriendlyName() const;
	friend class AtikCamera;
};

} // namespace atik
} // namespace camera
} // namespace astro

#endif /* _AtikFilterwheel_h */
