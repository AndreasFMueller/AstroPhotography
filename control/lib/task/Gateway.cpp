/*
 * Gateway.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <AstroCallback.h>

namespace astro {
namespace task {

Gateway::gatewaymap_t	Gateway::_taskupdates;
callback::CallbackPtr	Gateway::_callback;

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

void	Gateway::updateImageStart(const std::string& instrument) {
	if (instrument.size() == 0) { return; }
	TaskUpdatePtr	taskupdate = get(instrument);
	time(&taskupdate->lastimagestart);
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

} // namespace task
} // namespace astro
