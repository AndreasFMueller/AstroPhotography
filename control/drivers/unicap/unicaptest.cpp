/*
 * unicaptest.cpp -- class containing unit tests for the SBIG driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroIO.h>
#include <UnicapLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>

using namespace astro::camera::unicap;
using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {
namespace unicap {
namespace test {

class unicaptest : public CppUnit::TestFixture {
	UnicapCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();
	void	testCamera();
	void	testCcd();
	void	testExposure();

	CPPUNIT_TEST_SUITE(unicaptest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST(testCamera);
	CPPUNIT_TEST(testCcd);
	CPPUNIT_TEST(testExposure);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(unicaptest);

void	unicaptest::setUp() {
	locator = new UnicapCameraLocator();
}

void	unicaptest::tearDown() {
	delete locator;
	locator = NULL;
}

void	unicaptest::testList() {
	std::vector<std::string>	cameras = locator->getDevicelist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		std::cout << "camera[" << ++counter << "]: '" << *i << "'" << std::endl;
	}
}

void	unicaptest::testCamera() {
	CameraPtr	camera = locator->getCamera(0);
}

void	unicaptest::testCcd() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
}

void	unicaptest::testExposure() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	Exposure	exposure(ImageRectangle(ImagePoint(80, 50),
		ImageSize(500, 400)), 0.02);
	ccd->startExposure(exposure);
	ccd->exposureStatus();
	ShortImagePtr	image = ccd->shortImage();
	// write the image to a file
	FITSoutfile<unsigned short>	file("test.fits");
	file.write(*image);
}

} // namespace test
} // namespace unicap
} // namespace camera
} // namespace astro
