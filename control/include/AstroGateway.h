/*
 * AstroGateway.h -- task status gatewa
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroGateway_h
#define _AstroGateway_h

#include <AstroCamera.h>
#include <AstroCallback.h>
#include <AstroUtils.h>
#include <AstroCoordinates.h>

namespace astro {
namespace gateway {

/**
 * \brief Task update data structure to be sent to external monitors
 */
class TaskUpdate {
	std::string	_instrument;
public:
	const std::string& 	instrument() const { return _instrument; }
	TaskUpdate(const std::string& instrument);
	TaskUpdate(const TaskUpdate& other);
	TaskUpdate&	operator=(const TaskUpdate& other);
	time_t	updatetime;
	float	avgguideerror;
	// cooler
	float	ccdtemperature;
	time_t	lastimagestart;
	// exposure
	float	exposuretime;
	// taskqueue
	int	currenttaskid;
	// mount
	RaDec	telescope;
	bool	west;
	// filterwheel
	int	filter;
	LongLat	observatory;
	std::string	project;
	std::string	toString(std::string separator = std::string(" ")) const;
	operator	PostData() const;
};

typedef std::shared_ptr<TaskUpdate>	TaskUpdatePtr;

typedef callback::CallbackDataEnvelope<TaskUpdate> TaskUpdateCallbackData;

/**
 * \brief Gateway functions
 *
 * These static functions allow to collect information about the system
 * which can then be sent to a callback with the send function
 */
class Gateway {
public:
	static void	setCallback(callback::CallbackPtr callback);
	static bool	has(const std::string& instrument);
	static TaskUpdatePtr	get(const std::string& instrument);
	static void	update(const std::string& instrument,
				device::MountPtr mount);
	static void	update(const std::string& instrument,
				camera::CoolerPtr cooler);
	static void	update(const std::string& instrument,
				camera::FilterWheelPtr filterwheel);
	static void	update(const std::string& instrument,
				const camera::Exposure& exposure);
	static void	update(const std::string& instrument,
				float avgguideerror);
	static void	update(const std::string& instrument,
				int currenttaskid);
	static void	update(const std::string& instrument,
				const astro::Point& offset);
	static void	updateImageStart(const std::string& instrument);
	static void	update(const std::string& instrument,
				const std::string& project);
	static void	send(const std::string& instrument);
};

} // namespace task
} // namespace astro

#endif /* _AstroGateway_h */
