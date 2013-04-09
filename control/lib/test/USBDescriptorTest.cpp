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
	std::vector<DevicePtr>	devicelist = context.devices();
	int	ndevices = devicelist.size();
	CPPUNIT_ASSERT(ndevices > 0);

	std::vector<DevicePtr>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << **i << std::endl;
		DeviceDescriptorPtr	dd = (*i)->descriptor();
		std::cout << *dd << std::endl;
		for (int config = 0; config < dd->bNumConfigurations();
			config++) {
			ConfigurationPtr	c = (*i)->config(config);
			std::cout << *c;
		}
	}
}

CPPUNIT_TEST_SUITE_REGISTRATION(USBDescriptorTest);

} // namespace test
} // namespace astro
