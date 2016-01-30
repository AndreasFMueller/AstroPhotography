/*
 * ConvolutionAdpterTest.cpp -- template for tests
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
#include <math.h>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace test {

class ConvolutionAdapterTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testDisk();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ConvolutionAdapterTest);
	CPPUNIT_TEST(testDisk);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConvolutionAdapterTest);

void	ConvolutionAdapterTest::setUp() {
}

void	ConvolutionAdapterTest::tearDown() {
}

void	ConvolutionAdapterTest::testDisk() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisk() begin");

	Image<double>	psf(ImageSize(7,7));
	double	sigma2 = 3;
	for (int x = -3; x <= 3; x++) {
		for (int y = -3; y <= 3; y++) {
			psf.pixel(3 + x, 3 + y) = 
				exp(-(x*x+y*y)/(2 * sigma2)) / sqrt(2 * M_PI * sigma2);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "[%d,%d] = %f", x, y,
				psf.pixel(x + 3, y + 3));
		}
	}
	
	io::FITSin	imagefile("testimages/gauss.fits");
	ImagePtr	inptr = imagefile.read();
	Image<unsigned char>	*in
		= dynamic_cast<Image<unsigned char> *>(&*inptr);
	if (NULL == in) {
		throw std::runtime_error("not the right type of image");
	}
	LuminanceAdapter<unsigned char, double>	la(*in);

	ConvolutionAdapter<double>	ca(la, psf);
	ImagePtr	convolved(new Image<double>(ca));

	io::FITSout out("tmp/gauss-convolve.fits");
	out.setPrecious(false);
        out.write(convolved);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisk() end");
}

#if 0
void	ConvolutionAdapterTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
