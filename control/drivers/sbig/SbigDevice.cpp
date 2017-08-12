/*
 * SbigDevice.cpp
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <SbigDevice.h>
#include <SbigLock.h>
#include <utils.h>

namespace astro {
namespace camera {
namespace sbig {

SbigDevice::SbigDevice(SbigCamera& _camera) : camera(_camera) {
}

void	SbigDevice::query_command_status(QueryCommandStatusParams *params,
		QueryCommandStatusResults *results) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "query command status");
	SbigLock	lock;
	camera.sethandle();
        short   e = SBIGUnivDrvCommand(CC_QUERY_COMMAND_STATUS,
                params, results);
        if (e != CE_NO_ERROR) {
                debug(LOG_ERR, DEBUG_LOG, 0, "cannot query command status: %s",
                        sbig_error(e).c_str());
                throw SbigError(e);
        }
}

} // namespace sbig
} // namespace camera
} // namespace astro
