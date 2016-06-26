/*
 * AsiLocator.h -- declarations, 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

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
	static std::vector<bool>	cameraopen;
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
	static std::vector<std::string>	imgtypes(int index);
	static bool	isopen(int index);
	friend class AsiCamera;
	static void	initialize_cameraopen();
private:
	static void	setopen(int index, bool o);
	void	addCameraNames(std::vector<std::string>& names);
	void	addCcdNames(std::vector<std::string>& names);
	void	addCoolerNames(std::vector<std::string>& names);
	void	addGuiderportNames(std::vector<std::string>& names);
};

} // namespace asi
} // namespace camera
} // namespace astro

