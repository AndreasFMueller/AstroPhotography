/*
 * Mock1Test.cpp -- tests for the mock1 module
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */

#include <AstroLoader.h>
#include <AstroDevice.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <unistd.h>
#include <AstroDebug.h>

using namespace astro::device;
using namespace astro::camera;
using namespace astro::module;
using namespace astro::image;

namespace astro {
namespace test {

class Mock1Test : public CppUnit::TestFixture {
	ModuleRepositoryPtr	repository;
public:
	void	setUp();
	void	tearDown();
	void	testMock1();

	CPPUNIT_TEST_SUITE(Mock1Test);
	CPPUNIT_TEST(testMock1);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Mock1Test);

void	Mock1Test::setUp() {
	repository = getModuleRepository("../drivers/mock1/.libs");
}

void	Mock1Test::tearDown() {
	repository.reset();
}

void	Mock1Test::testMock1() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Mock1Test begin");
	ModulePtr	module = repository->getModule("mock1");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got module");
	module->open();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "module open");
	DeviceLocatorPtr	cl = module->getDeviceLocator();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get DeviceLocator");
	std::vector<std::string>	cameras = cl->getDevicelist();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get %d devices", cameras.size());
	CPPUNIT_ASSERT(cameras.size() == 10);
	CameraPtr	camera = cl->getCamera("camera:mock1/5");
	// for every CCD, take an image
	for (unsigned int i = 0; i < camera->nCcds(); i++) {
		CcdPtr	ccd = camera->getCcd(i);
		Exposure	exposure;
		ImageRectangle	frame(ImagePoint(1,1),
			ImageSize(ccd->getSize().width() - 2,
			ccd->getSize().height() - 2));
		exposure.frame(frame);
		ccd->startExposure(exposure);
		while (ccd->exposureStatus() == CcdState::exposing) {
			sleep(1);
		}
		if (ccd->exposureStatus() == CcdState::exposed) {
			ImagePtr	image = ccd->getImage();
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"result image size: %d x %d",
				image->size().width(), image->size().height());
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Mock1Test end");
}

} // namespace test
} // namespace astro
