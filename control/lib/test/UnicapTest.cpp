/*
 * UnicapTest.cpp -- tests for the Unicap classes
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroUnicap.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <stdexcept>
#include <debug.h>
#include <iomanip>

using namespace astro::unicap;

namespace astro {
namespace test {

class UnicapTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testEnumeration();
	void	testGet();
	void	testDeviceInfo();
	void	testFormats();
	void	testProperties();
	void	testCapture();

	CPPUNIT_TEST_SUITE(UnicapTest);
	CPPUNIT_TEST(testEnumeration);
	CPPUNIT_TEST(testGet);
	CPPUNIT_TEST(testDeviceInfo);
	CPPUNIT_TEST(testFormats);
	CPPUNIT_TEST(testProperties);
	CPPUNIT_TEST(testCapture);
	CPPUNIT_TEST_SUITE_END();
};

void	UnicapTest::testEnumeration() {
	try {
		Unicap	unicap;
		std::cout << "devices: " << unicap.numDevices() << std::endl;
	} catch (std::exception& x) {
		std::cerr << "exception: " << x.what() << std::endl;
	}
}

void	UnicapTest::testGet() {
	Unicap	unicap;
	UnicapDevice	device = unicap.get(0);
}

void	UnicapTest::testDeviceInfo() {
	Unicap	unicap;
	for (int i = 0; i < unicap.numDevices(); i++) {
		std::cout << "Device " << i << std::endl;
		UnicapDevice	device = unicap.get(i);
		std::cout << "Identifier:     ";
		std::cout << device.identifier() << std::endl;
		std::cout << "Model name:     ";
		std::cout << device.model_name() << std::endl;
		std::cout << "Vendor name:    ";
		std::cout << device.vendor_name() << std::endl;
	}
}

void	UnicapTest::testFormats() {
	Unicap	unicap;
	int	ndevices = unicap.numDevices();
	for (int d = 0; d < ndevices; d++) {
		UnicapDevice	device = unicap.get(d);
		int	n = device.numFormats();
		std::cout << "number of formats: " << n << std::endl;
		for (int i = 0; i < n; i++) {
			UnicapFormat	format = device.getFormat(i);
			std::cout << "format " << i << ": " << format.identifier();
			std::cout << "sizes: " << format.numSizes() << std::endl;
			for (int j = 0; j < format.numSizes(); j++) {
				UnicapRectangle	r = format.get(j);
				std::cout << "   [" << j << "]: " << r.width()
					<< " x " << r.height() << std::endl;
			}
		}
	}
}

void	UnicapTest::testProperties() {
	Unicap	unicap;
	int	ndevices = unicap.numDevices();
	for (int d = 0; d < ndevices; d++) {
		UnicapDevice	device = unicap.get(d);
		std::cout << device << std::endl;
		int	n = device.numProperties();
		std::cout << "    number of properties: " << n << std::endl;
		for (int i = 0; i < n; i++) {
			UnicapPropertyPtr	property
				= device.getProperty(i);
			std::cout << "        " << *property << std::endl;
		}
	}
}

void	UnicapTest::testCapture() {
	debuglevel = LOG_DEBUG;
	Unicap	unicap;
	UnicapDevice	device = unicap.get(0);
	std::cout << "Device: " << device.identifier() << std::endl;
	device.numFormats();
	UnicapFormat	format = device.getFormat(0);
	std::cout << "Format: " << format.identifier() << std::endl;
	device.setFormat(0);
	device.getFrames(10);
}

CPPUNIT_TEST_SUITE_REGISTRATION(UnicapTest);

} // namespace test
} // namespace astro
