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

static void	showInterfaceDescriptor(const InterfaceDescriptor& ifdesc) {
	std::string	extra = ifdesc.extra();
	if (extra.size() > 0) {
		std::cout << "extra size: " << extra.size() << std::endl;
		UVCDescriptorFactory	f(ifdesc.device());
		std::cout << f.descriptors(extra.c_str(), extra.size());
	}
}

static void	showInterfaceList(const std::vector<Interface>& iflist) {
	std::vector<Interface>::const_iterator	i;
	for (i = iflist.begin(); i != iflist.end(); i++) {
		for (int j = 0; j < i->numAltsettings(); j++) {
			showInterfaceDescriptor((*i)[j]);
		}
	}
}

static void	showConfigDescriptorExtra(const ConfigDescriptor& config) {
	DescriptorFactory	f(config.device());
	std::vector<USBDescriptorPtr>	l = f.descriptors(config.extra().c_str(), config.extra().size());
	std::vector<USBDescriptorPtr>::const_iterator	i;
	for (i = l.begin(); i != l.end(); i++) {
		std::cout << *i;
	}
}

void	UVCDescriptorTest::testList() {
	Context	context;
	context.setDebugLevel(0);
	std::vector<Device>	devicelist = context.devices();
	std::vector<Device>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << *i << std::endl;
		DeviceDescriptor	*dd = i->descriptor();
		for (int config = 0; config < dd->bNumConfigurations();
			config++) {
			ConfigDescriptor	*c = i->config(config);
			showConfigDescriptorExtra(*c);

			const std::vector<Interface>&	iflist = c->interface();
			showInterfaceList(iflist);

			delete c;
		}
		delete dd;
	}
	CPPUNIT_ASSERT(devicelist.size() > 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(UVCDescriptorTest);

} // namespace test
} // namespace astro
