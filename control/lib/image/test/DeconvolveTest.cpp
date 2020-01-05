/*
 * DeconvolveTest.cpp -- template for tests
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroConvolve.h>
#include <AstroAdapter.h>
#include <AstroIO.h>

using namespace astro::image;

namespace astro {
namespace test {

class DeconvolveTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testDisk();
	void	testGauss();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(DeconvolveTest);
	CPPUNIT_TEST(testDisk);
	CPPUNIT_TEST(testGauss);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DeconvolveTest);

void	DeconvolveTest::setUp() {
}

void	DeconvolveTest::tearDown() {
}

void	DeconvolveTest::testDisk() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisk() begin");
	// build the image
	DiskImage	image1(ImageSize(400,300), ImagePoint(47,62), 10, 1);
	ImagePtr	iptr = ImagePtr(new Image<double>(image1));

	// build the psf
	DiskImage	psf(ImageSize(100,100), ImagePoint(50,50), 10, 1);

	// deconvolve
	FourierDeconvolutionOperator	decon(psf);
	ImagePtr	fq = decon(iptr);
	
	io::FITSout out("tmp/deconvolve-disk.fits");
	out.setPrecious(false);
        out.write(fq);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisk() end");
}

void	DeconvolveTest::testGauss() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGauss() begin");
	double	n = 200;
	Image<float>	*image = new Image<float>(400, 300);
	for (int x = 0; x < 400; x++) {
		for (int y = 0; y < 300; y++) {
			double	r = hypot(x - 247, y - 128);
			image->pixel(x, y) = expf(-r * r / n);
		}
	}
	ImagePtr	imgptr(image);
	io::FITSout	outimage("tmp/deconvolve-gaussimage.fits");
	outimage.setPrecious(false);
	outimage.write(imgptr);

	Image<double>	*psf = new Image<double>(100, 100);
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 100; y++) {
			double	r = hypot(x - 50, y - 50);
			psf->pixel(x, y) = exp(-r * r / n);
		}
	}
	ImagePtr	psfptr(psf);

	FourierDeconvolutionOperator	decon(psfptr);
	ImagePtr	deconvolved = decon(imgptr);

	io::FITSout	out("tmp/deconvolve-gauss.fits");
	out.setPrecious(false);
	out.write(deconvolved);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGauss() end");
}

#if 0
void	DeconvolveTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
