/*
 * UVCDescriptorTest.cpp -- tests for the usb::Context class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroUSB.h>
#include <AstroUVC.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>

using namespace astro::usb;
using namespace astro::usb::uvc;

namespace astro {
namespace test {

class UVCDescriptorTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testList();

	CPPUNIT_TEST_SUITE(UVCDescriptorTest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST_SUITE_END();
};

static void	showInterfaceDescriptor(Device& device, InterfaceDescriptorPtr ifdesc) {
	std::string	extra = ifdesc->extra();
	if (extra.size() > 0) {
		std::cout << "extra size: " << extra.size() << std::endl;
		UVCDescriptorFactory	f(device);
		std::cout << f.descriptors(extra.c_str(), extra.size());
	}
}

static void	showInterface(Device& device, const InterfacePtr& interface) {
	for (int j = 0; j < interface->numAltsettings(); j++) {
		showInterfaceDescriptor(device, (*interface)[j]);
	}
}

static void	showConfigurationExtra(Device& device, const Configuration& config) {
	DescriptorFactory	f(device);
	std::vector<USBDescriptorPtr>	l = f.descriptors(config.extra().c_str(), config.extra().size());
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		std::cout << *i;
	}
}

void	UVCDescriptorTest::testList() {
	Context	context;
	context.setDebugLevel(0);
	std::vector<DevicePtr>	devicelist = context.devices();
	std::vector<DevicePtr>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << **i << std::endl;
		DeviceDescriptorPtr	dd = (*i)->descriptor();
		for (int config = 0; config < dd->bNumConfigurations();
			config++) {
			ConfigurationPtr	c = (*i)->config(config);
			showConfigurationExtra(**i, *c);

			for (int ifno = 0; ifno < c->bNumInterfaces(); ifno++) {
				showInterface(**i, (*c)[ifno]);
			}
		}
	}
	CPPUNIT_ASSERT(devicelist.size() > 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(UVCDescriptorTest);

} // namespace test
} // namespace astro
