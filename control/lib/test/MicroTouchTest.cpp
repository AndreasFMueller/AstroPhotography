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
	Context	context;
	context.setDebugLevel(3);
	DevicePtr	deviceptr = context.find(0x10c4, 0x82f4);
	std::cout << *deviceptr;

	// open the device as a 
	try {
		MicroTouch	microtouch(*deviceptr);
		std::cout << "microtouch initialized " << std::endl;
		microtouch.stepUp();
		std::cout << "microtouch stepped " << std::endl;
	} catch (std::exception& x) {
		std::cerr << "exception: " << x.what() << std::endl;
	}
}

CPPUNIT_TEST_SUITE_REGISTRATION(MicroTouchTest);

} // namespace test
} // namespace astro
