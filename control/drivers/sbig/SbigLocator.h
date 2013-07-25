/*
 * SbigLocator.h -- declarations, 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroLoader.h>
#include <AstroCamera.h>
#include <includes.h>
#include <pthread.h>

using namespace astro::camera;

namespace astro {
namespace camera {
namespace sbig {

/**
 * \brief The SBIG CameraLocator class.
 *
 * The SBIG library provides methods to list cameras, this is just
 * an adapter class to the CameraLocator class.
 */
class SbigCameraLocator : public CameraLocator {
public:
	SbigCameraLocator();
	virtual ~SbigCameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual	std::vector<std::string>	getCameralist();
	virtual CameraPtr	getCamera(const std::string& name);
	virtual CameraPtr	getCamera(size_t index);
};

/**
 * \brief Locking class for SBIG camera driver
 */
class SbigLock {
public:
	SbigLock();
	~SbigLock();
};

} // namespace sbig
} // namespace camera
} // namespace astro

