/*
 * CameraTest.cpp -- tests the nice camera driver
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroLoader.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <AstroDebug.h>
#include <AstroIO.h>

namespace astro {
namespace test {

class NiceCameraTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testCamera();
	CPPUNIT_TEST_SUITE(NiceCameraTest);
	CPPUNIT_TEST(testCamera);
	CPPUNIT_TEST_SUITE_END();
};

extern "C" astro::module::ModuleDescriptor *getDescriptor();
extern "C" astro::device::DeviceLocator	*getDeviceLocator();

void	NiceCameraTest::testCamera() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCamera() begin");
	astro::module::Repository	repo;
	astro::module::Devices	devices(repo);
	DeviceName	devicename("camera:nice/server/simulator/camera");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "getting simulator camera: %s",
		devicename.toString().c_str());
	astro::camera::CameraPtr	camera = devices.getCamera(devicename);
	CPPUNIT_ASSERT(camera->nCcds() == 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera name: %s",
		camera->name().toString().c_str());
	astro::camera::CcdPtr	ccd = camera->getCcd(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "ccd name: %s",
		ccd->name().toString().c_str());
	astro::camera::Exposure	exposure;
	exposure.exposuretime = 4;
	exposure.frame = ccd->getInfo().getFrame();
	exposure.shutter = astro::camera::Shutter::OPEN;
	exposure.purpose = astro::camera::Exposure::light;
	ccd->startExposure(exposure);
	ccd->wait();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "status: %d", ccd->exposureStatus());
	astro::image::ImagePtr	image = ccd->getImage();
	astro::io::FITSout	out("test.fits");
	out.setPrecious(false);
	out.write(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCamera() end");
};

/*
void	NiceCameraTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
*/

CPPUNIT_TEST_SUITE_REGISTRATION(NiceCameraTest);

} // namespace test
} // namespace astro
