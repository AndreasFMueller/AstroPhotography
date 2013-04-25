/*
 * sxtest.cpp -- sx hardware tests
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <sxhw.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>

using namespace astro::sx;

namespace astro {
namespace sx {
namespace test {

class sxtest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testVersion();
	void	testReset();
	void	testClear();
	void	testGetCcdParams();
	void	testModel();
	void	testTimer();

        CPPUNIT_TEST_SUITE(sxtest);
        CPPUNIT_TEST(testConstructor);
        CPPUNIT_TEST(testVersion);
        CPPUNIT_TEST(testReset);
        CPPUNIT_TEST(testClear);
        CPPUNIT_TEST(testGetCcdParams);
        CPPUNIT_TEST(testModel);
        //CPPUNIT_TEST(testTimer);
        CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sxtest);

void	sxtest::setUp() {
}

void	sxtest::tearDown() {
}

void	sxtest::testConstructor() {
	Context	context;
	DevicePtr	deviceptr = context.find(0x1278, 0x0326);
	std::cout << *(deviceptr->activeConfig());
	SxCamera	camera(*deviceptr);
}

void	sxtest::testVersion() {
	Context	context;
	DevicePtr	deviceptr = context.find(0x1278, 0x0326);
	SxCamera	camera(*deviceptr);
	sx_firmware_version_t	version = camera.getVersion();
	CPPUNIT_ASSERT(version.major_version == 1);
	CPPUNIT_ASSERT(version.minor_version == 17);
}

void	sxtest::testReset() {
	Context	context;
	DevicePtr	deviceptr = context.find(0x1278, 0x0326);
	SxCamera	camera(*deviceptr);
	camera.reset();
}

void	sxtest::testClear() {
	Context	context;
	DevicePtr	deviceptr = context.find(0x1278, 0x0326);
	SxCamera	camera(*deviceptr);
	camera.clear((uint16_t)0);
}

void	sxtest::testGetCcdParams() {
	Context	context;
	DevicePtr	deviceptr = context.find(0x1278, 0x0326);
	SxCamera	camera(*deviceptr);
	sx_ccd_params_t	params = camera.getCcdParams((int16_t)0);
	std::cout << "hfront_porch:       ";
	std::cout << (int)params.hfront_porch << std::endl;
	std::cout << "hback_porch:        ";
	std::cout << (int)params.hback_porch << std::endl;
	std::cout << "width:              ";
	std::cout << (int)params.width << std::endl;
	std::cout << "vfront_porch:       ";
	std::cout << (int)params.vfront_porch << std::endl;
	std::cout << "vback_porch:        ";
	std::cout << (int)params.vback_porch << std::endl;
	std::cout << "height:             ";
	std::cout << (int)params.height << std::endl;
	std::cout << "pixel_uwidth:       ";
	std::cout << (params.pixel_uwidth / 256.) << std::endl;
	std::cout << "pixel_uheight:      ";
	std::cout << (params.pixel_uheight / 256.) << std::endl;
	std::cout << "color:              ";
	std::cout << (int)params.color << std::endl;
	std::cout << "bits_per_pixel:     ";
	std::cout << (int)params.bits_per_pixel << std::endl;
	std::cout << "num_serial_ports:   ";
	std::cout << (int)params.num_serial_ports << std::endl;
	std::cout << "extra_capabilities: ";
	std::cout << std::hex << (int)params.extra_capabilities << std::endl;
}

void	sxtest::testModel() {
	Context	context;
	DevicePtr	deviceptr = context.find(0x1278, 0x0326);
	SxCamera	camera(*deviceptr);
	uint16_t	model = camera.getModel();
	std::cout << "model: " << model << std::endl;
}

void	sxtest::testTimer() {
	Context	context;
	DevicePtr	deviceptr = context.find(0x1278, 0x0326);
	SxCamera	camera(*deviceptr);
	camera.setTimer(1000);
	uint32_t	t = camera.getTimer();
	std::cout << "timer: " << t << std::endl;
	camera.setTimer(0);
}

} // namespace test
} // namespace sx
} // namespace astro
