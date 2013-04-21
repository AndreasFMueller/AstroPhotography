/*
 * MicroTouchTest.cpp -- tests for the usb::Context class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroUSB.h>
#include <MicroTouch.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <stdexcept>
#include <debug.h>

using namespace astro::usb;
using namespace astro::microtouch;

namespace astro {
namespace test {

class MicroTouchTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testPosition();

	CPPUNIT_TEST_SUITE(MicroTouchTest);
	CPPUNIT_TEST(testPosition);
	CPPUNIT_TEST_SUITE_END();
};

void	MicroTouchTest::testPosition() {
	//debuglevel = LOG_DEBUG;
	Context	context;
	context.setDebugLevel(1);
	try {
		DevicePtr	deviceptr = context.find(0x10c4, 0x82f4);
		std::cout << *deviceptr->activeConfig();

		// open the device as a MicroTouch
		MicroTouch	microtouch(*deviceptr);
		std::cout << "microtouch initialized " << std::endl;

		microtouch.setPosition(1);

		// find the current position
		while (1) {
		std::cout << "position: ";
		std::cout << (int)microtouch.position() << std::endl;
		sleep(1);

		//std::cout << "getWord(0x94):  ";
		//std::cout << (int)microtouch.getWord(0x94) << std::endl;

		std::cout << "getWord(0x9a):  ";
		std::cout << (int)microtouch.getWord(0x9a) << std::endl;

		std::cout << "getWord(0x9c):  ";
		std::cout << (int)microtouch.getWord(0x9c) << std::endl;

		std::cout << "getWord(0xa0):  ";
		std::cout << (int)microtouch.getWord(0xa0) << std::endl;

		std::cout << "getWord(0x9e):  ";
		std::cout << (int)microtouch.getWord(0x9e) << std::endl;

		std::cout << "getWord(0x92):  ";
		std::cout << (int)microtouch.getWord(0x92) << std::endl;

		std::cout << "temperature compensation:  ";
		std::cout << (microtouch.isTemperatureCompensating() ? "ON" : "OFF") << std::endl;

		std::cout << "getByte(0x92):  ";
		std::cout << (int)microtouch.getByte(0x92) << std::endl;

		std::cout << "temperature:    ";
		std::cout << microtouch.getTemperature() << std::endl;

		std::cout << "moving:         ";
		std::cout << ((microtouch.isMoving()) ? "moving" : "not moving") << std::endl;
		std::cout << std::endl;
		}
	
		// step up
		//microtouch.stepUp();

		std::cout << "microtouch stepped " << std::endl;
	} catch (std::exception& x) {
		std::cerr << "exception: " << x.what() << std::endl;
	}
}

CPPUNIT_TEST_SUITE_REGISTRATION(MicroTouchTest);

} // namespace test
} // namespace astro
