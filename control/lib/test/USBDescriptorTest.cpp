/*
 * USBDescriptorTest.cpp -- tests for the usb::Context class
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

class USBDescriptorTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testList();
	void	testAllconfigs();

	CPPUNIT_TEST_SUITE(USBDescriptorTest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST(testAllconfigs);
	CPPUNIT_TEST_SUITE_END();
};

void	USBDescriptorTest::testList() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testList() begin");
	Context	context;
	context.setDebugLevel(3);
	std::vector<DevicePtr>	devicelist = context.devices();
	int	ndevices = devicelist.size();
	CPPUNIT_ASSERT(ndevices > 0);

	std::vector<DevicePtr>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << **i << std::endl;
		try {
			(*i)->open();
			DeviceDescriptorPtr	dd = (*i)->descriptor();
			std::cout << *dd << std::endl;
			ConfigurationPtr	c = (*i)->activeConfig();
			std::cout << *c;
			(*i)->close();
		} catch (std::exception& x) {
			std::cout << "cannot display device: " << x.what()
				<< std::endl;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testList() end");
}

void	USBDescriptorTest::testAllconfigs() {
return;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAllconfigs() begin");
	Context	context;
	context.setDebugLevel(0);
	std::vector<DevicePtr>	devicelist = context.devices();
	int	ndevices = devicelist.size();
	CPPUNIT_ASSERT(ndevices > 0);

	std::vector<DevicePtr>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << **i << std::endl;
		try {
			DeviceDescriptorPtr	dd = (*i)->descriptor();
			(*i)->open();
			std::cout << *dd << std::endl;
			for (int config = 1; config <= dd->bNumConfigurations();
				config++) {
				ConfigurationPtr	c = (*i)->config(config);
				std::cout << *c;
			}
			(*i)->close();
		} catch (std::exception& x) {
			std::cout << "cannot show all configs: " << x.what() << std::endl;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAllconfigs() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(USBDescriptorTest);

} // namespace test
} // namespace astro
