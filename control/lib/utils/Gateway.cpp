/*
 * Gateway.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroGateway.h>
#include <AstroCallback.h>

namespace astro {
namespace gateway {

class ExponentialMovingAverage {
	float	_alpha;
	float	_avg;
public:
	float	avg() const { return _avg; }
	float	alpha() const { return _alpha; }
	ExponentialMovingAverage(float alpha = 0.9) : _alpha(alpha), _avg(0) { }
	void	add(float s) {
		_avg = _alpha * s + (1 - _alpha) * _avg;
	}
};

typedef std::shared_ptr<ExponentialMovingAverage> ExponentialMovingAveragePtr;
typedef std::map<std::string, ExponentialMovingAveragePtr>	averagemap_t;
static averagemap_t	_averages;

typedef std::map<std::string, TaskUpdatePtr>	taskupdatemap_t;
static taskupdatemap_t	_taskupdates;

static callback::CallbackPtr	_callback;

void	Gateway::setCallback(callback::CallbackPtr callback) {
	_callback = callback;
}

bool	Gateway::has(const std::string& instrument) {
	if (instrument.size() == 0) { return false; }
	return (_taskupdates.find(instrument) != _taskupdates.end());
}

TaskUpdatePtr	Gateway::get(const std::string& instrument) {
	if (instrument.size() == 0) { return TaskUpdatePtr(); }
	if (!has(instrument)) {
		TaskUpdate	*taskupdate = new TaskUpdate(instrument);
		_taskupdates.insert(std::make_pair(instrument,
			TaskUpdatePtr(taskupdate)));
		ExponentialMovingAverage	*average
			= new ExponentialMovingAverage();
		_averages.insert(std::make_pair(instrument,
			ExponentialMovingAveragePtr(average)));
	}
	return _taskupdates.find(instrument)->second;
}

void	Gateway::update(const std::string& instrument,
		const camera::Exposure& exposure) {
	if (instrument.size() == 0) { return; }
	TaskUpdatePtr	taskudpate = get(instrument);
	taskudpate->exposuretime = exposure.exposuretime();
}

void	Gateway::update(const std::string& instrument,
		device::MountPtr mount) {
	if (instrument.size() == 0) { return; }
	if (!mount) { return; }
	TaskUpdatePtr	taskupdate = get(instrument);
	try {
		taskupdate->updatetime = mount->time();
		taskupdate->telescope = mount->getRaDec();
		taskupdate->observatory = mount->location();
		taskupdate->west = mount->telescopePositionWest();
	} catch (const std::exception& ex) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get mount info: %s",
			ex.what());
	}
}

void	Gateway::update(const std::string& instrument,
		camera::CoolerPtr cooler) {
	if (instrument.size() == 0) { return; }
	if (!cooler) { return; }
	TaskUpdatePtr	taskupdate = get(instrument);
	try {
		taskupdate->ccdtemperature = cooler->getActualTemperature();
	} catch (const std::exception& ex) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get temperature: %s",
			ex.what());
	}
}

void	Gateway::update(const std::string& instrument,
		camera::FilterWheelPtr filterwheel) {
	if (instrument.size() == 0) { return; }
	if (!filterwheel) { return; }
	TaskUpdatePtr	taskupdate = get(instrument);
	try {
		taskupdate->filter = filterwheel->currentPosition();
	} catch (const std::exception& ex) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get filter: %s",
			ex.what());
	}
}

void	Gateway::update(const std::string& instrument,
		float avgguideerror) {
	if (instrument.size() == 0) { return; }
	TaskUpdatePtr	taskupdate = get(instrument);
	taskupdate->avgguideerror = avgguideerror;
}

void	Gateway::update(const std::string& instrument,
		int currenttaskid) {
	if (instrument.size() == 0) { return; }
	TaskUpdatePtr	taskupdate = get(instrument);
	taskupdate->currenttaskid = currenttaskid;
}

void	Gateway::update(const std::string& instrument,
		const Point& offset) {
	if (instrument.size() == 0) { return; }
	ExponentialMovingAveragePtr	average
		= _averages.find(instrument)->second;
	average->add(offset.abs());
	TaskUpdatePtr	taskupdate = get(instrument);
	taskupdate->avgguideerror = average->avg();
}

void	Gateway::updateImageStart(const std::string& instrument) {
	if (instrument.size() == 0) { return; }
	TaskUpdatePtr	taskupdate = get(instrument);
	time_t	now;
	time(&now);
	taskupdate->lastimagestart = now;
}

void	Gateway::send(const std::string& instrument) {
	if (!_callback) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no callback installed");
		return;
	}
	if (instrument.size() == 0) {
		return;
	}
	TaskUpdate	tup = *get(instrument);
	TaskUpdateCallbackData	*cdb = new TaskUpdateCallbackData(tup);
	callback::CallbackDataPtr	data(cdb);
	try {
		(*_callback)(data);
	} catch (const std::exception& ex) {
		debug(LOG_ERR, DEBUG_LOG, 0, "failed to send the update: %s",
			ex.what());
	}
}

} // namespace gateway
} // namespace astro
