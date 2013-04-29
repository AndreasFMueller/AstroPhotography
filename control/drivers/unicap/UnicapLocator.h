/*
 * UnicapLocator.h -- declarations of the camera 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _UnicapLocator_h
#define _UnicapLocator_h

using namespace astro::camera;

namespace astro {
namespace camera {
namespace unicap {

/**
 * \brief The Unicap camera locator
 *
 * Each Unicap camera is also a camera from the point of view of this 
 */
class UnicapCameraLocator : public CameraLocator {
public:
	UnicapCameraLocator();
	virtual ~UnicapCameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getModule() const;
	virtual std::vector<std::string>	getCameralist();
	virtual CameraPtr	getCamera(const std::string& name);
	virtual CameraPtr	getCamera(size_t index);
};

} // namespace unicap
} // namespace camera
} // namespace astro

#endif /* _UnicapLocator_h */
