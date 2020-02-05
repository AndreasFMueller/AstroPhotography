/*
 * DitherWork.cpp
 *
 * (c) 2019 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroTask.h>
#include <ExposureWork.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <unistd.h>
#include <AstroGuiding.h>
#include <AstroDiscovery.h>

namespace astro {
namespace task {

/**
 * \brief Condition object that can decide when a dither operation is complete
 */
class DitherCondition : public Condition {
	astro::guiding::GuiderPtr	_guider;
public:
	DitherCondition(astro::guiding::GuiderPtr guider) : _guider(guider) { }
	// this currently does nothing, so the dither timeout will kick in
	bool	operator()() {
		return false;
	}
};

/**
 * \brief Construct a DitherWork object
 */
DitherWork::DitherWork(TaskQueueEntry& task) : TaskWork(task) {
	if (task.taskType() != tasktype(tasktype::DITHER)) {
		std::string	msg = stringprintf("%d is not a dither task",
			task.id());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::logic_error(msg);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "construct dither task work object %s",
		task.toString().c_str());
}

/**
 * \brief Destroy the DitherWork object
 */
DitherWork::~DitherWork() {
}

/**
 * \brief Do the sleep work, i.e. do sleep
 */
void	DitherWork::run() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start to dither task");
	try {
		// Get the default guider descriptor from the instrument
		std::string	iname = task().instrument();
		discover::InstrumentPtr	instrument
			= discover::InstrumentBackend::get(iname);
		guiding::GuiderDescriptor	guiderdescriptor
			= instrument->guiderdescriptor();

		// the instrument must also be able to build guider
		// descriptor. We need this for the GuiderFactory is based
		// on the GuiderDescriptor rather than the name

		// infer the dither amount from the task parameters
		// the ccd temperature is abused for the dither pixel
		double	arcsec = task().ccdtemperature().temperature();
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dithering for %.1f arcsec",
			arcsec);

		// Get the Guider that we can infer from the Instrument
		// currently in use. For this step, we need access to the
		// guiderfactory, but there is currently no static method
		// for that.
		guiding::GuiderPtr	guider
			= guiding::GuiderFactory::get()->get(guiderdescriptor);
		if (!guider) {
			throw std::runtime_error("no guider");
		}

		// Send the dither command to the guider
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dithering for %f arcsec",
			arcsec);
		guider->ditherArcsec(arcsec);

		// get the maximum time to wait from the exposure time
		debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up dither condition");
		DitherCondition	dithercondition(guider);
		double	maxwaittime = task().exposure().exposuretime();
		if (maxwaittime <= 0) {
			maxwaittime = 15.;
		}

		// now wait for the guiding condition to be satisfied again
		debug(LOG_DEBUG, DEBUG_LOG, 0, "waiting for dither");
		if (!wait(maxwaittime, dithercondition)) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"dither condition not met");
		}
	} catch (CancelException& cancelexception) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "dither %d task cancelled",
			task().id());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end dither task");
}

} // namespace task
} // namespace astro
