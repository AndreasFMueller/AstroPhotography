/*
 * NiceTest.cpp -- test the nice and denice classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <AstroDebug.h>
#include <Nice.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace astro {
namespace test {

class NiceTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testNice();
	void	testDenice();

	CPPUNIT_TEST_SUITE(NiceTest);
	CPPUNIT_TEST(testNice);
	CPPUNIT_TEST(testDenice);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NiceTest);

void	NiceTest::setUp() {
}

void	NiceTest::tearDown() {
}

void	NiceTest::testNice() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNice() begin");
	device::nice::DeviceNicer	nicer("server");
	DeviceName	name("camera:sx/1-2-3/camera");
	DeviceName	newname = nicer(name);
	CPPUNIT_ASSERT(newname.modulename() == "nice");
	CPPUNIT_ASSERT(newname.hasType(DeviceName::Camera));
	CPPUNIT_ASSERT(newname.unitname() == "camera");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNice() end");
}

void	NiceTest::testDenice() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDenice() begin");
	DeviceName	name("camera:nice/server/sx/1-2-3/camera");
	device::nice::DeviceDenicer	denicer(name);
	CPPUNIT_ASSERT(denicer.service() == "server");
	DeviceName	newname = denicer.devicename();
	CPPUNIT_ASSERT(newname.modulename() == "sx");
	CPPUNIT_ASSERT(newname.unitname() == "camera");
	CPPUNIT_ASSERT(newname.hasType(DeviceName::Camera));
	CPPUNIT_ASSERT(newname.toString() == "camera:sx/1-2-3/camera");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDenice() end");
}


} // namespace test
} // namespace astro
