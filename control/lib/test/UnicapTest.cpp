/*
 * UnicapTest.cpp -- tests for the Unicap classes
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroUnicap.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <stdexcept>
#include <debug.h>
#include <iomanip>

using namespace astro::unicap;
using namespace astro::image;
using namespace astro::usb;
using namespace astro::io;

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEnumeration() begin");
	try {
		Unicap	unicap;
		std::cout << "devices: " << unicap.numDevices() << std::endl;
	} catch (std::exception& x) {
		std::cerr << "exception: " << x.what() << std::endl;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEnumeration() end");
}

void	UnicapTest::testGet() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGet() begin");
	Unicap	unicap;
	UnicapDevice	device = unicap.get(0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGet() end");
}

void	UnicapTest::testDeviceInfo() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDeviceInfo() begin");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDeviceInfo() end");
}

void	UnicapTest::testFormats() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFormats() begin");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFormats() end");
}

void	UnicapTest::testProperties() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testProperties() begin");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testProperties() end");
}

void	UnicapTest::testCapture() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCapture() begin");
	debuglevel = LOG_DEBUG;
	Unicap	unicap;
	UnicapDevice	device = unicap.get(0);
	std::cout << "Device: " << device.identifier() << std::endl;
	device.numFormats();
	UnicapFormat	format = device.getFormat(0);
	std::cout << "Format: " << format << std::endl;
	device.setFormat(format);
	std::vector<FramePtr>	frames = device.getFrames(10);
	std::vector<FramePtr>::const_iterator	i;
	int	count = 0;
	for (i = frames.begin(); i != frames.end(); i++) {
		int	width = (*i)->getWidth();
		int	height = (*i)->getHeight();
		Image<YUYV<unsigned char> >     *image =
			new Image<YUYV<unsigned char> >(width, height);
		const char	*data = (*i)->data();
		int	size = (*i)->size();
		for (int j = 0; 2 * j < size; j++) {
			(*image)[j] = YUYV<unsigned char>(data[2 * j],
				data[2 * j + 1]);
		}
		char	buffer[128];
		snprintf(buffer, sizeof(buffer), "out%d.fits", count++);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "out file: %s", buffer);
		std::string	filename(buffer);
		FITSoutfile<YUYV<unsigned char> >	file(filename);
		file.write(*image);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCapture() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(UnicapTest);

} // namespace test
} // namespace astro
