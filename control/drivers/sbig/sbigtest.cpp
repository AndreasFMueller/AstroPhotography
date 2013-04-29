/*
 * sbigtest.cpp -- class containing unit tests for the SBIG driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroIO.h>
#include <SbigLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>

using namespace astro::camera::sbig;
using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {
namespace sbig {
namespace test {

class sbigtest : public CppUnit::TestFixture {
	SbigCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();
	void	testCamera();
	void	testCcd();
	void	testExposure();

	CPPUNIT_TEST_SUITE(sbigtest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST(testCamera);
	CPPUNIT_TEST(testCcd);
	CPPUNIT_TEST(testExposure);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sbigtest);

void	sbigtest::setUp() {
	locator = new SbigCameraLocator();
}

void	sbigtest::tearDown() {
	delete locator;
	locator = NULL;
}

void	sbigtest::testList() {
	std::vector<std::string>	cameras = locator->getCameralist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		std::cout << "camera[" << ++counter << "]: '" << *i << "'" << std::endl;
	}
}

void	sbigtest::testCamera() {
	CameraPtr	camera = locator->getCamera(0);
}

void	sbigtest::testCcd() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	std::cout << ccd->getInfo() << std::endl;
}

void	sbigtest::testExposure() {
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(0);
	Exposure	exposure(ImageRectangle(ImagePoint(80, 50),
		ImageSize(500, 400)), 0.02);
	//Exposure	exposure(ImageRectangle(ImagePoint(0, 0),
	//	ImageSize(648, 486)), 0.02);
	//Exposure	exposure(ImageRectangle(ImagePoint(80, 200),
	//	ImageSize(500, 286)), 0.02);
	std::cout << exposure << std::endl;
	ccd->startExposure(exposure);
	ccd->exposureStatus();
	ShortImagePtr	image = ccd->shortImage();
	// write the image to a file
	FITSoutfile<unsigned short>	file("test.fits");
	file.write(*image);
}

} // namespace test
} // namespace sbig
} // namespace camera
} // namespace astro
