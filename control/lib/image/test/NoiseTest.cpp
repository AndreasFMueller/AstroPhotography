/*
 * NoiseTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroFilterfunc.h>
#include <AstroAdapter.h>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace test {

class NoiseTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testAverage();
	void	testSBIG16803();
	void	testGaussNoise();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(NoiseTest);
	CPPUNIT_TEST(testAverage);
	CPPUNIT_TEST(testSBIG16803);
	CPPUNIT_TEST(testGaussNoise);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(NoiseTest);

void	NoiseTest::setUp() {
}

void	NoiseTest::tearDown() {
}

void	NoiseTest::testAverage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAverage() begin");
	ImageSize	size(2000, 2000);
	DarkNoiseAdapter	darknoise(size, 273.13, 100, 1000);
	ImagePtr	dark(new Image<double>(darknoise));
	double	m = astro::image::filter::mean(dark);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mean: %f", m);
	CPPUNIT_ASSERT(fabs(m - 0.1) < 0.01);
	io::FITSout	out("tmp/darknoise.fits");
	out.setPrecious(false);
	out.write(dark);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAverage() end");
}

void	NoiseTest::testSBIG16803() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSBIG16803() begin");
	ImageSize	size(4096, 4096);
	double	darkcurrent = 0.02;
	double	exposuretime = 600;
	int	electrons_per_pixel = 100000;
	DarkNoiseAdapter	darknoise(size, 273.13,
		darkcurrent * exposuretime, electrons_per_pixel);
	double	expected = darkcurrent * exposuretime / electrons_per_pixel;
	ImagePtr	dark(new Image<double>(darknoise));
	double	m = astro::image::filter::mean(dark);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mean: %f", m);
	CPPUNIT_ASSERT(fabs(m - expected) < 0.01);
	io::FITSout	out("tmp/sbignoise.fits");
	out.setPrecious(false);
	out.write(dark);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSBIG16803() end");
}

void	NoiseTest::testGaussNoise() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGaussNoise() begin");
	ImageSize	size(1000,1000);
	double	mu = 0.1;
	double	sigma = 0.001;
	GaussNoiseAdapter	gaussnoise(size, mu, sigma);
	ImagePtr	gauss(new Image<double>(gaussnoise));
	double	m = astro::image::filter::mean(gauss);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mean: %f", m);
	CPPUNIT_ASSERT(fabs(m - mu) < 0.01);
	io::FITSout	out("tmp/gaussnoise.fits");
	out.setPrecious(false);
	out.write(gauss);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGaussNoise() end");
}

#if 0
void	NoiseTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
