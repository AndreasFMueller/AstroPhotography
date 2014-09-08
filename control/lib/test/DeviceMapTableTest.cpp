/*
 * DeviceMapTableTest.cpp -- Test the devicemap configuraiton variables table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <includes.h>
#include <DeviceMapTable.h>

using namespace astro::persistence;
using namespace astro::config;

namespace astro {
namespace test {

class DeviceMapTableTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testDeviceMap();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(DeviceMapTableTest);
	CPPUNIT_TEST(testDeviceMap);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DeviceMapTableTest);

void	DeviceMapTableTest::setUp() {
}

void	DeviceMapTableTest::tearDown() {
}

void	DeviceMapTableTest::testDeviceMap() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDeviceMap() begin");
	unlink("devicemaptest.db");
	Database	database = DatabaseFactory::get("devicemaptest.db");
	DeviceMapTable	devicemap(database);
	DeviceMapRecord	devicemap1;
	devicemap1.name = "devicemap1";
	devicemap1.description = "Description of devicemap1";
	devicemap1.servername = "localhost";
	devicemap1.devicename = "camera:simulator/camera";
	long	id1 = devicemap.add(devicemap1);
	DeviceMapRecord	devicemap2;
	devicemap2.name = "devicemap2";
	devicemap2.description = "Description of devicemap2";
	devicemap2.servername = "titus";
	devicemap2.devicename = "camera:simulator/camera";
	long	id2 = devicemap.add(devicemap2);
	DeviceMapRecord	devicemap3 = devicemap.byid(id1);
	CPPUNIT_ASSERT(devicemap1.name == devicemap3.name);
	CPPUNIT_ASSERT(devicemap1.description == devicemap3.description);
	DeviceMapRecord	devicemap4 = devicemap.byid(id2);
	CPPUNIT_ASSERT(devicemap2.name == devicemap4.name);
	CPPUNIT_ASSERT(devicemap2.description == devicemap4.description);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDeviceMap() end");
}

#if 0
void	DeviceMapTableTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
