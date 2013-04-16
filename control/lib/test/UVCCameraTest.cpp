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
#include <stdexcept>

using namespace astro::usb;
using namespace astro::usb::uvc;

namespace astro {
namespace test {

class UVCCameraTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testCameraList();
	void	testCamera();

	CPPUNIT_TEST_SUITE(UVCCameraTest);
	//CPPUNIT_TEST(testCameraList);
	CPPUNIT_TEST(testCamera);
	CPPUNIT_TEST_SUITE_END();
};

void	UVCCameraTest::testCameraList() {
	Context	context;
	context.setDebugLevel(0);
	std::vector<DevicePtr>	devicelist = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << *i << std::endl;
		try {
			UVCCamera	camera(**i);
			std::cout << camera;
		} catch (std::exception& x) {
			std::cout << "not a camera: " << x.what() << std::endl;
		}
	}
	CPPUNIT_ASSERT(devicelist.size() > 0);
}

void	UVCCameraTest::testCamera() {
	Context	context;
	context.setDebugLevel(3);
	//DevicePtr	deviceptr = context.find(0x199e, 0x8101); // TIS
	DevicePtr	deviceptr = context.find(0x046d, 0x082b); // Logitech
	//DevicePtr	deviceptr = context.find(0x0c45, 0x6340); // Sonix
	//DevicePtr	deviceptr = context.find(0x04fc, 0x2001); // Sunplus
	std::cout << *deviceptr;

	// open the device as a 
	try {
		UVCCamera	camera(*deviceptr, true);
		std::cout << camera;
		std::cout << "select FormatAndFrame" << std::endl;
		camera.selectFormatAndFrame(1, 1, 1);
		std::pair<uint8_t, uint8_t>	ff
			= camera.getFormatAndFrame(1);
		std::cout << "format " << (int)ff.first
			<< ", frame = " << (int)ff.second << std::endl;
		camera.getFrame(1);
	} catch (std::exception& x) {
		std::cerr << "cannot build camera: " << x.what() << std::endl;
	}
}

CPPUNIT_TEST_SUITE_REGISTRATION(UVCCameraTest);

} // namespace test
} // namespace astro
