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
	//void	testXXX();

	CPPUNIT_TEST_SUITE(DeconvolveTest);
	CPPUNIT_TEST(testDisk);
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
	DiskImage	image1(ImageSize(256,256), ImagePoint(47,62), 1, 1);
	ImagePtr	iptr = ImagePtr(new Image<double>(image1));

	DiskImage	image2(ImageSize(256,256), ImagePoint(128,111), 1, 1);
	BasicDeconvolutionOperator	decon(image2);
	ImagePtr	fq = decon(iptr);
	
	io::FITSout out("tmp/deconvolve-disk.fits");
	out.setPrecious(false);
        out.write(fq);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisk() end");
}

#if 0
void	DeconvolveTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
