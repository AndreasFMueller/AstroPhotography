/*
 * uvctest.cpp -- class containing unit tests for the SBIG driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroIO.h>
#include <UVCLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>
#include <debug.h>

using namespace astro::camera::uvc;
using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {
namespace uvc {
namespace test {

class uvctest : public CppUnit::TestFixture {
	UVCCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();
	void	testCamera();
	void	testCcd();
	void	testExposure();

	CPPUNIT_TEST_SUITE(uvctest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST(testCamera);
	CPPUNIT_TEST(testCcd);
	CPPUNIT_TEST(testExposure);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(uvctest);

void	uvctest::setUp() {
	locator = new UVCCameraLocator();
}

void	uvctest::tearDown() {
	delete locator;
	locator = NULL;
}

void	uvctest::testList() {
	std::vector<std::string>	cameras = locator->getCameralist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		std::cout << "camera[" << ++counter << "]: '" << *i << "'"
			<< std::endl;
	}
}

void	uvctest::testCamera() {
	CameraPtr	camera = locator->getCamera(0);
	std::cout << "number of ccds: " << camera->nCcds() << std::endl;
}

void	uvctest::testCcd() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCcd");
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	std::cout << ccd->getInfo() << std::endl; 
}

void	uvctest::testExposure() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	Exposure	exposure(ImageRectangle(ImagePoint(0, 0),
		ImageSize(160, 120)), 0.02);
	ccd->startExposure(exposure);
	ccd->exposureStatus();
	ImagePtr	image = ccd->getImage();
	// write the image to a file
	FITSout	file("test.fits");
	file.write(image);
}

} // namespace test
} // namespace uvc
} // namespace camera
} // namespace astro
