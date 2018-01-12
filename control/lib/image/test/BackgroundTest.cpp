/*
 * BackgroundTests.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroBackground.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace test {

class BackgroundTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testBase();

	CPPUNIT_TEST_SUITE(BackgroundTest);
	CPPUNIT_TEST(testBase);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BackgroundTest);

void	BackgroundTest::setUp() {
}

void	BackgroundTest::tearDown() {
}

void	BackgroundTest::testBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBase() begin");
	// create a linear function
	LinearFunction	lf(ImagePoint(1000, 500), false);
	lf[0] = 0.01;
	lf[1] = 0.02;
	lf[2] = 47;

	// create an image with random data in it
	unsigned int	width = 2000;
	unsigned int	height = 1000;
	Image<float>	image(ImageSize(width, height));
	for (unsigned int x = 0; x < width; x++) {
		for (unsigned int y = 0; y < height; y++) {
			double	error = -0.5 + random() / (double)2147483647;
			float	value = lf(x, y) + error;
			//debug(LOG_DEBUG, DEBUG_LOG, 0, "%f", value);
			image.pixel(x, y) = value;
		}
	}

	// compute the lower bound
	MinimumEstimator<LinearFunction>	me(std::map<std::string, double>(), image, 100);
	FunctionPtr	l2 = me(ImagePoint(1000, 500), false);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBase() end");
}

} // namespace test
} // namespace astro
