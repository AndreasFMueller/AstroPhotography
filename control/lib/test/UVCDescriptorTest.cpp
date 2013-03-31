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

static void	showInterfaceDescriptorList(const std::list<InterfaceDescriptor>&ifdlist) {
	std::list<InterfaceDescriptor>::const_iterator i;
	for (i = ifdlist.begin(); i != ifdlist.end(); i++) {
		std::string	extra = i->extra();
		if (extra.size() > 0) {
			std::cout << "extra size: " << extra.size() << std::endl;
			UVCDescriptorFactory	f(i->device());
			std::cout << f.descriptors(extra.c_str(), extra.size());
		}
	}
}

static void	showInterfaceList(const std::list<Interface>& iflist) {
	std::list<Interface>::const_iterator	i;
	for (i = iflist.begin(); i != iflist.end(); i++) {
		const std::list<InterfaceDescriptor>&	ifdlist
			= i->altsettings();
		showInterfaceDescriptorList(ifdlist);
	}
}

void	UVCDescriptorTest::testList() {
	Context	context;
	context.setDebugLevel(0);
	std::list<Device>	devicelist = context.list();
	std::list<Device>::const_iterator	i;
	for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << *i << std::endl;
		DeviceDescriptor	*dd = i->descriptor();
		for (int config = 0; config < dd->bNumConfigurations();
			config++) {
			ConfigDescriptor	*c = i->config(config);
			const std::list<Interface>&	iflist = c->interface();
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
