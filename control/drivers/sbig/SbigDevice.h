/*
 * SbigDevice.h -- common stuff associated with the reference to the camera
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _SbigDevice_h
#define _SbigDevice_h

#include <SbigCamera.h>
#include <includes.h>

namespace astro {
namespace camera {
namespace sbig {

class SbigDevice {
protected:
	SbigCamera	&camera;
	void	query_command_status(QueryCommandStatusParams *params,
        		QueryCommandStatusResults *results);
public:
	SbigDevice(SbigCamera& _camera);
};

} // namespace sbig
} // namespace camera
} // namespace astro

#endif /* _SbigDevice_h */
