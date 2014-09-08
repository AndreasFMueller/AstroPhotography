/*
 * DeviceMapperTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroPersistence.h>
#include <includes.h>

using namespace astro::config;
using namespace astro::persistence;

namespace astro {
namespace test {

class DeviceMapperTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testAdd();
	void	testFind();
	void	testUpdate();
	void	testRemove();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(DeviceMapperTest);
	CPPUNIT_TEST(testAdd);
	CPPUNIT_TEST(testFind);
	CPPUNIT_TEST(testUpdate);
	CPPUNIT_TEST(testRemove);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DeviceMapperTest);

void	DeviceMapperTest::setUp() {
}

void	DeviceMapperTest::tearDown() {
}

void	DeviceMapperTest::testAdd() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAdd() begin");
	unlink("devicemappertest.db");
	Database	db = DatabaseFactory::get("devicemappertest.db");
	DeviceMapperPtr	devicemapper = DeviceMapper::get(db);

	DeviceName	cameraname("camera:simulator/camera");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating a map entry for '%s'",
		cameraname.toString().c_str());
	DeviceMap	mapentry1(cameraname);
	mapentry1.name("SIM");
	mapentry1.servername("");
	mapentry1.description("mapper entry for the simulator camera");
	devicemapper->add(mapentry1);

	cameraname = DeviceName("camera:sx/camera1");
	DeviceMap	mapentry2(cameraname);
	mapentry2.name("SX");
	mapentry2.servername("titus");
	mapentry2.description("mapper entry for the SX camera");
	devicemapper->add(mapentry2);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAdd() end");
}

void	DeviceMapperTest::testFind() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFind() begin");

	Database	db = DatabaseFactory::get("devicemappertest.db");
	DeviceMapperPtr	devicemapper = DeviceMapper::get(db);

	// find the SIM camera
	DeviceMap	mapentry1 = devicemapper->find("SIM");
	DeviceName	cameraname("camera:simulator/camera");
	CPPUNIT_ASSERT(mapentry1.devicename() == cameraname);
	CPPUNIT_ASSERT(mapentry1.servername() == "");
	CPPUNIT_ASSERT(mapentry1.description()
		== "mapper entry for the simulator camera");
	
	cameraname = DeviceName("camera:sx/camera1");
	DeviceMap	mapentry2 = devicemapper->find(cameraname, "titus");
	CPPUNIT_ASSERT(mapentry2.name() == "SX");
	CPPUNIT_ASSERT(mapentry2.devicename() == cameraname);
	CPPUNIT_ASSERT(mapentry2.servername() == "titus");
	CPPUNIT_ASSERT(mapentry2.description()
		== "mapper entry for the SX camera");
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFind() end");
}

void	DeviceMapperTest::testUpdate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testUpdate() begin");
	Database	db = DatabaseFactory::get("devicemappertest.db");
	DeviceMapperPtr	devicemapper = DeviceMapper::get(db);

	// find the SIM camera
	DeviceMap	mapentry1 = devicemapper->find("SIM");
	DeviceName	cameraname("camera:simulator/camera2");
	mapentry1.devicename(cameraname);
	devicemapper->update("SIM", mapentry1);
	
	DeviceMap	mapentry2 = devicemapper->find("SIM");
	CPPUNIT_ASSERT(mapentry2.devicename() == cameraname);
	CPPUNIT_ASSERT(mapentry2.servername() == "");
	CPPUNIT_ASSERT(mapentry2.description() == 
		"mapper entry for the simulator camera");

	// update the SX camera
	DeviceMap	mapentry3
		= devicemapper->find(DeviceName("camera:sx/camera1"), "titus");
	mapentry3.name("SY");
	devicemapper->update(DeviceName("camera:sx/camera1"), "titus",
		mapentry3);
	
	DeviceMap	mapentry4 = devicemapper->find("SY");
	CPPUNIT_ASSERT(mapentry4.devicename()
		== DeviceName("camera:sx/camera1"));
	CPPUNIT_ASSERT(mapentry4.servername() == "titus");
	CPPUNIT_ASSERT(mapentry4.description()
		== "mapper entry for the SX camera");

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testUpdate() end");
}

void	DeviceMapperTest::testRemove() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRemove() begin");
	
	Database	db = DatabaseFactory::get("devicemappertest.db");
	DeviceMapperPtr	devicemapper = DeviceMapper::get(db);
	devicemapper->remove("SY");
	try {
		devicemapper->find("SY");
	} catch (std::exception& x) {
	}
	devicemapper->remove(DeviceName("camera:simulator/camera2"), "");
	try {
		devicemapper->find("SIM");
	} catch (std::exception& x) {
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRemove() end");
}

#if 0
void	DeviceMapperTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
