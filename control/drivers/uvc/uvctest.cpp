/*
 * uvctest.cpp -- class containing unit tests for the SBIG driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroIO.h>
#include <UvcLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>
#include <debug.h>
#include <AstroDemosaic.h>

extern double	default_exposuretime;
extern int	default_ccdid;

using namespace astro::camera::uvc;
using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace camera {
namespace uvc {
namespace test {

class uvctest : public CppUnit::TestFixture {
	UvcCameraLocator	*locator;
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
	locator = new UvcCameraLocator();
}

void	uvctest::tearDown() {
	delete locator;
	locator = NULL;
}

void	uvctest::testList() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create a list of cameras");
	std::vector<std::string>	cameras = locator->getDevicelist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		std::cout << "camera[" << ++counter << "]: '" << *i << "'"
			<< std::endl;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "cameras listed");
}

void	uvctest::testCamera() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the first camera");
	CameraPtr	camera = locator->getCamera(0);
	std::cout << "number of ccds: " << camera->nCcds() << std::endl;
	for (unsigned int i = 0; i < camera->nCcds(); i++) {
		std::cout << camera->getCcdInfo(i) << std::endl;
	}
}

void	uvctest::testCcd() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCcd");
	CameraPtr	camera = locator->getCamera(0);
	CcdPtr	ccd = camera->getCcd(default_ccdid);
	std::cout << ccd->getInfo() << std::endl; 
}

void	uvctest::testExposure() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the first camera device");
	CameraPtr	camera = locator->getCamera(0);
	int	ccdindex = default_ccdid;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "get the CCD no %d", ccdindex);
	CcdPtr	ccd = camera->getCcd(ccdindex);
	Exposure	exposure(ccd->getInfo().getFrame(),
		default_exposuretime);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start an exposure: %s",
		exposure.toString().c_str());
	ccd->startExposure(exposure);
	ccd->exposureStatus();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieve an image");
	ImageSequence	imgseq = ccd->getImageSequence(2);
	ImagePtr	image = imgseq[imgseq.size() - 1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image retrieved");
	// write the image to a file
	unlink("test.fits");
	FITSout	file("test.fits");
	file.write(image);

	if (ccdindex == 2) {
		DemosaicBilinear<unsigned char>        demosaicer;
		Image<unsigned char>	*mosaicimg
			= dynamic_cast<Image<unsigned char> *>(&*image);
		if (NULL != mosaicimg) {
			Image<RGB<unsigned char> >     *demosaiced
				= demosaicer(*mosaicimg);
			ImagePtr        demosaicedptr(demosaiced);
			unlink("test-demosaiced.fits");
			FITSout demosaicedfile("test-demosaiced.fits");
			demosaicedfile.write(demosaicedptr);
		} else {
			debug(LOG_ERR, DEBUG_LOG, 0, "not a mosaic image");
		}
	}
}

} // namespace test
} // namespace uvc
} // namespace camera
} // namespace astro
