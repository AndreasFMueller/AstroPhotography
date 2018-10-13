/*
 * AsiLocator.h -- declarations, 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AsiLocator_h
#define _AsiLocator_h

#include <AstroLoader.h>
#include <AstroCamera.h>
#include <AstroDevice.h>
#include <includes.h>

using namespace astro::camera;
using namespace astro::device;

namespace astro {
namespace camera {
namespace asi {

class AsiCamera;

/**
 * \brief The SBIG CameraLocator class.
 *
 * The SBIG library provides methods to list cameras, this is just
 * an adapter class to the CameraLocator class.
 */
class AsiCameraLocator : public DeviceLocator {
	std::recursive_mutex	_mutex;
	static std::vector<bool>	cameraopen;
	static void	initialize_cameraopen();
public:
	AsiCameraLocator();
	virtual ~AsiCameraLocator();
	virtual std::string	getName() const;
	virtual std::string	getVersion() const;
	virtual	std::vector<std::string>	getDevicelist(DeviceName::device_type device = DeviceName::Camera);
protected:
	virtual CameraPtr	getCamera0(const DeviceName& name);
public:
	// stuff that is available to open cameras
	std::vector<std::string>	imgtypes(int index);
	bool	isopen(int index);
	friend class AsiCamera;
private:
	void	setopen(int index, bool o);
	void	addCameraNames(std::vector<std::string>& names);
	void	addCcdNames(std::vector<std::string>& names);
	void	addCoolerNames(std::vector<std::string>& names);
	void	addGuideportNames(std::vector<std::string>& names);
};

} // namespace asi
} // namespace camera
} // namespace astro

#endif /* _AsiLocator_h */
