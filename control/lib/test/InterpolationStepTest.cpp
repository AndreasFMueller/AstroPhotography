/*
 * InterpolationStepTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroProcess.h>
#include <limits>
#include <list>

using namespace astro::image;
using namespace astro::process;

namespace astro {
namespace test {

class InterpolationStepTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testInterpolate();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(InterpolationStepTest);
	CPPUNIT_TEST(testInterpolate);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(InterpolationStepTest);

void	InterpolationStepTest::setUp() {
}

void	InterpolationStepTest::tearDown() {
}

static double	pixelvalue(unsigned int x, unsigned int y) {
	return M_PI * x + M_E * y;
}

static double	pixelvalue(const ImagePoint& point) {
	return pixelvalue(point.x(), point.y());
}

void	InterpolationStepTest::testInterpolate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInterpolate() begin");
	ImageSize	size(360, 240);
	ProcessingController	controller;

	// create an image with with linear pixel values
	Image<double>	*image = new Image<double>(size);
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			image->writablepixel(x, y) = pixelvalue(x, y);
		}
	}
	ImagePtr	imageptr = ImagePtr(image);

	// make some pixels bad
	std::list<ImagePoint>	badpixels;
	int	counter = 10;
	while (counter--) {
		ImagePoint	point(random() % (size.width() / 2), random() % size.height());
		badpixels.push_back(point);
		image->writablepixel(point) = std::numeric_limits<double>::quiet_NaN();
	}

	unsigned int	xb = 3 * size.width() / 4;
	// make a bad vertical line
	for (unsigned int y = 10; y < 20; y++) {
		image->writablepixel(xb, y) = std::numeric_limits<double>::quiet_NaN();
	}

	// make a bad horizontal line
	for (unsigned int dx = 0; dx < 10; dx++) {
		image->writablepixel(xb + dx, 30) = std::numeric_limits<double>::quiet_NaN();
	}
	
	// make a 3x3 block of pixels bad
	for (unsigned int x = xb; x < xb + 3; x++) {
		for (unsigned int y = 40; y < 40 + 3; y++) {
			image->writablepixel(x, y) = std::numeric_limits<double>::quiet_NaN();
		}
	}

	// build an RawImageStep from it
	ProcessingStepPtr	imagestep = ProcessingStepPtr(new RawImageStep(imageptr));
	controller.addstep("image", imagestep);

	// add the interpolation step
	InterpolationStep	*interpol = new InterpolationStep(1);
	ProcessingStepPtr	interpolationstep = ProcessingStepPtr(interpol);
        controller.addstep("interpolate", interpolationstep);
	controller.add_precursor("interpolate", "image");
	
	// do the interpolation
	controller.execute(2);
	CPPUNIT_ASSERT(interpolationstep->status() == ProcessingStep::complete);

	// test the result
	std::list<ImagePoint>::const_iterator	i;
	for (i = badpixels.begin(); i != badpixels.end(); i++) {
		ImagePoint	point = *i;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "check bad pixel at %s",
			point.toString().c_str());
		double	v = interpol->out().pixel(point);
		double	v2 = pixelvalue(point);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "@%s: %f ?= %f", point.toString().c_str(), v, v2);
		CPPUNIT_ASSERT(fabs(v - v2) < 0.01);
	}

	// check the vertical line
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking vertical line");
	for (unsigned int y = 11; y < 19; y++) {
		double	v = interpol->out().pixel(xb, y);
		double	v2 = pixelvalue(xb, y);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "@(%u,%u): %f ?= %f", xb, y, v2);
		CPPUNIT_ASSERT(fabs(v - v2) < 0.01);
	}
	{
		double	v = interpol->out().pixel(xb, 10);
		double	v2 = pixelvalue(xb, 10);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "@(%u,%u): %f close to %f", xb, 10, v, v2);
		CPPUNIT_ASSERT(fabs(v - v2) < 0.01 + M_E / 3.);
	}
	{
		double	v = interpol->out().pixel(xb, 19);
		double	v2 = pixelvalue(xb, 19);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "@(%u,%u): %f close to %f", xb, 19, v, v2);
		CPPUNIT_ASSERT(fabs(v - v2) < 0.01 + M_E / 3.);
	}

	// check the horizontal line
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking horizontal line");
	for (unsigned int dx = 1; dx < 9; dx++) {
		double	v = interpol->out().pixel(xb + dx, 30);
		double	v2 = pixelvalue(xb + dx, 30);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "@(%u,%u): %f ?= %f", xb, 30, v, v2);
		CPPUNIT_ASSERT(fabs(v - v2) < 0.01);
	}
	{
		double	v = interpol->out().pixel(xb, 30);
		double	v2 = pixelvalue(xb, 30);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "@(%u,%u): %f close to %f", xb, 30, v, v2);
		CPPUNIT_ASSERT(fabs(v - v2) <= 0.01 + M_PI / 3);
	}
	{
		double	v = interpol->out().pixel(xb + 9, 30);
		double	v2 = pixelvalue(xb + 9, 30);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "@(%u,%u): %f close to %f", xb, 30, v, v2);
		CPPUNIT_ASSERT(fabs(v - v2) <= 0.01 + M_PI / 3);
	}

	// check the block
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking block");
	for (unsigned int x = xb; x < xb + 3; x++) {
		for (unsigned int y = 40; y < 40 + 3; y++) {
			if ((x == xb + 1) && (y == 41)) {
				continue;
			}
			double	v = interpol->out().pixel(x, y);
			double	v2 = pixelvalue(x, y);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "check point %u,%u: %f close to %f?", x, y, v, v2);
			CPPUNIT_ASSERT(v == v);
			CPPUNIT_ASSERT(fabs(v - pixelvalue(x, y)) < 0.01 + M_PI);
		}
	}
	double	v = interpol->out().pixel(xb + 1, 41);
	CPPUNIT_ASSERT(v != v); // central pixel cannot be interpolated

	// that's it
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInterpolate() end");
}

#if 0
void	InterpolationStepTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
