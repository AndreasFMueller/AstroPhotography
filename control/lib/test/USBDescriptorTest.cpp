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

using namespace astro::usb;

namespace astro {
namespace test {

class USBDescriptorTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testList();

	CPPUNIT_TEST_SUITE(USBDescriptorTest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST_SUITE_END();
};

void	USBDescriptorTest::testList() {
	Context	context;
	context.setDebugLevel(0);
	std::list<Device>	devicelist = context.list();
	std::list<Device>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << *i << std::endl;
		DeviceDescriptor	*dd = i->descriptor();
		std::cout << *dd << std::endl;
		for (int config = 0; config < dd->bNumConfigurations();
			config++) {
			ConfigDescriptor	*c = i->config(config);
			std::cout << *c;
		}
		delete dd;
	}
	CPPUNIT_ASSERT(devicelist.size() > 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(USBDescriptorTest);

} // namespace test
} // namespace astro
