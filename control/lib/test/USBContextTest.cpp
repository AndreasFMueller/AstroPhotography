/*
 * USBContextTest.cpp -- tests for the usb::Context class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroUSB.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>

using namespace astro::usb;

namespace astro {
namespace test {

class USBContextTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testBasic();
	void	testLowDebugLevel();
	void	testHighDebugLevel();
	void	testList();

	CPPUNIT_TEST_SUITE(USBContextTest);
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST_EXCEPTION(testLowDebugLevel, std::range_error);
	CPPUNIT_TEST_EXCEPTION(testHighDebugLevel, std::range_error);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST_SUITE_END();
};

void	USBContextTest::testLowDebugLevel() {
	Context	context;
	context.setDebugLevel(-1);
}

void	USBContextTest::testHighDebugLevel() {
	Context	context;
	context.setDebugLevel(4);
}

void	USBContextTest::testBasic() {
	Context	context;
	context.setDebugLevel(3);
}

void	USBContextTest::testList() {
	Context	context;
	context.setDebugLevel(0);
	std::list<Device>	devicelist = context.list();
	std::list<Device>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << *i << std::endl;
	}
	CPPUNIT_ASSERT(devicelist.size() > 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(USBContextTest);

} // namespace test
} // namespace astro
