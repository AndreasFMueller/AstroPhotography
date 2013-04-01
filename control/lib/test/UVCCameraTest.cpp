/*
 * UVCCameraTest.cpp -- tests for the usb::Context class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroUSB.h>
#include <AstroUVC.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>

using namespace astro::usb;
using namespace astro::usb::uvc;

namespace astro {
namespace test {

class UVCCameraTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testList();

	CPPUNIT_TEST_SUITE(UVCCameraTest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST_SUITE_END();
};

void	UVCCameraTest::testList() {
	Context	context;
	context.setDebugLevel(0);
	std::vector<Device>	devicelist = context.devices();
	std::vector<Device>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << *i << std::endl;
		try {
			UVCCamera	camera(*i);
#if 0
			std::cout << camera;
#endif
		} catch (std::exception& x) {
			std::cout << "not a camera: " << x.what() << std::endl;
		}
	}
	CPPUNIT_ASSERT(devicelist.size() > 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(UVCCameraTest);

} // namespace test
} // namespace astro
