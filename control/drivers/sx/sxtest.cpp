/*
 * sxtest.cpp -- tests for the SX driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SxLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>
#include <debug.h>

namespace astro {
namespace camera {
namespace sx {
namespace test {

class sxtest : public CppUnit::TestFixture {
	static SxCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();
	void	testCamera();

	CPPUNIT_TEST_SUITE(sxtest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST(testCamera);
	CPPUNIT_TEST_SUITE_END();
};

SxCameraLocator	*sxtest::locator = NULL;

CPPUNIT_TEST_SUITE_REGISTRATION(sxtest);

void	sxtest::setUp() {
	if (NULL == locator) {
		locator = new SxCameraLocator();
	}
}

void	sxtest::tearDown() {
}

void	sxtest::testList() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices");
	std::vector<std::string>	cameras = locator->getCameralist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera[%d]: %s", ++counter,
			i->c_str());
	}
}

void	sxtest::testCamera() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices");
	std::vector<std::string>	cameras = locator->getCameralist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera[%d]: %s", ++counter,
			i->c_str());
	}
	CameraPtr	camera = locator->getCamera(*cameras.begin());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera has %d ccds", camera->nCcds());
	CcdPtr	ccd = camera->getCcd(0);
}

} // namespace test
} // namespace sx
} // namespace camera
} // namespace astro
