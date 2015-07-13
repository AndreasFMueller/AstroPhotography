/*
 * LocatorTest.cpp -- tests the NiceLocator class
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <NiceLocator.h>
#include <AstroLoader.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <AstroDebug.h>

namespace astro {
namespace test {

class NiceLocatorTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testModule();
	void	testLocator();
	void	testDeviceList();
	CPPUNIT_TEST_SUITE(NiceLocatorTest);
	CPPUNIT_TEST(testModule);
	CPPUNIT_TEST(testLocator);
	CPPUNIT_TEST(testDeviceList);
	CPPUNIT_TEST_SUITE_END();
};

extern "C" astro::module::ModuleDescriptor *getDescriptor();
extern "C" astro::device::DeviceLocator	*getDeviceLocator();

void	NiceLocatorTest::testModule() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testModule() begin");
	astro::module::ModuleDescriptor	*module = getDescriptor();
	CPPUNIT_ASSERT(module->name() == "nice");
	CPPUNIT_ASSERT(module->version() == VERSION);
	delete module;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testModule() end");
};

void	NiceLocatorTest::testLocator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testModule() begin");
	astro::device::DeviceLocator	*devicelocator = getDeviceLocator();
	CPPUNIT_ASSERT(devicelocator->getName() == "nice");
	CPPUNIT_ASSERT(devicelocator->getVersion() == VERSION);
	delete devicelocator;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testModule() end");
}

void	NiceLocatorTest::testDeviceList() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDeviceList() begin");
	astro::device::DeviceLocator	*devicelocator = getDeviceLocator();
	std::this_thread::sleep_for(std::chrono::seconds(5));
	std::vector<std::string>	list = devicelocator->getDevicelist();
	delete devicelocator;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDeviceList() end");
}

/*
void	NiceLocatorTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
*/

CPPUNIT_TEST_SUITE_REGISTRATION(NiceLocatorTest);

} // namespace test
} // namespace astro
