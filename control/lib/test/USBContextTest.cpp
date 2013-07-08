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
#include <debug.h>

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testLowDebugLevel() begin");
	Context	context;
	context.setDebugLevel(-1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testLowDebugLevel() end");
}

void	USBContextTest::testHighDebugLevel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testHighDebugLevel() begin");
	Context	context;
	context.setDebugLevel(4);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testHighDebugLevel() end");
}

void	USBContextTest::testBasic() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() begin");
	Context	context;
	context.setDebugLevel(3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() end");
}

void	USBContextTest::testList() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testList() begin");
	Context	context;
	context.setDebugLevel(0);
	std::vector<DevicePtr>	devicelist = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << **i << std::endl;
	}
	CPPUNIT_ASSERT(devicelist.size() > 0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testList() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(USBContextTest);

} // namespace test
} // namespace astro
