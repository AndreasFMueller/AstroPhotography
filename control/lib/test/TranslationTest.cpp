/**
 * TranslationTest.cpp -- 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <debug.h>
#include <iostream>

using namespace astro::image;
using namespace astro::image::transform;

namespace astro {
namespace test {

const static double	epsilon = 0.001;

class TranslationTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testPositiveTranslation();
	void	testNegativeTranslation();

	CPPUNIT_TEST_SUITE(TranslationTest);
	CPPUNIT_TEST(testPositiveTranslation);
	CPPUNIT_TEST(testNegativeTranslation);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TranslationTest);

void	TranslationTest::setUp() {
}

void	TranslationTest::tearDown() {
}

void	TranslationTest::testPositiveTranslation() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin translation test");
	// create an image
	Image<double>	image(4, 4);
	for (unsigned int x = 0; x < 4; x++) {
		for (unsigned int y = 0; y < 4; y++) {
			image.pixel(x, y) = 10 + 2 * x + y;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"image.pixel(%u, %u) = %f", x, y,
				image.pixel(x, y));
		}
	}

	// create a translation
	Point	translation(2.25, 1.333333);

	// create a translation adapter
	TranslationAdapter<double>	ta(image, translation);
	ImageSize	size = ta.getSize();
	for (unsigned int x = 0; x < 4; x++) {
		for (unsigned int y = 0; y < 4; y++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "ta.pixel(%u, %u) = %f",
				x, y, ta.pixel(x, y));
		}
	}

	// check pixel values
	CPPUNIT_ASSERT(fabs(ta.pixel(0, 0) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(0, 1) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(0, 2) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(0, 3) -  0.0000) < epsilon);

	CPPUNIT_ASSERT(fabs(ta.pixel(1, 0) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(1, 1) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(1, 2) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(1, 3) -  0.0000) < epsilon);

	CPPUNIT_ASSERT(fabs(ta.pixel(2, 0) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(2, 1) -  5.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(2, 2) -  8.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(2, 3) -  8.7500) < epsilon);

	CPPUNIT_ASSERT(fabs(ta.pixel(3, 0) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(3, 1) -  7.6666) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(3, 2) - 12.1666) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(3, 3) - 13.1666) < epsilon);

	// that's it
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation test complete");
}

void	TranslationTest::testNegativeTranslation() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin negative translation test");
	// create an image
	Image<double>	image(4, 4);
	for (unsigned int x = 0; x < 4; x++) {
		for (unsigned int y = 0; y < 4; y++) {
			image.pixel(x, y) = 10 + 2 * x + y;
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"image.pixel(%u, %u) = %f", x, y,
				image.pixel(x, y));
		}
	}

	// create a translation
	Point	translation(-2.25, -1.333333);

	// create a translation adapter
	TranslationAdapter<double>	ta(image, translation);
	ImageSize	size = ta.getSize();
	for (unsigned int x = 0; x < 4; x++) {
		for (unsigned int y = 0; y < 4; y++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "ta.pixel(%u, %u) = %f",
				x, y, ta.pixel(x, y));
		}
	}

	// check pixel values
	CPPUNIT_ASSERT(fabs(ta.pixel(0, 0) - 15.8333) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(0, 1) - 16.8333) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(0, 2) - 11.6666) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(0, 3) -  0.0000) < epsilon);

	CPPUNIT_ASSERT(fabs(ta.pixel(1, 0) - 13.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(1, 1) - 13.7500) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(1, 2) -  9.5000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(1, 3) -  0.0000) < epsilon);

	CPPUNIT_ASSERT(fabs(ta.pixel(2, 0) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(2, 1) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(2, 2) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(2, 3) -  0.0000) < epsilon);

	CPPUNIT_ASSERT(fabs(ta.pixel(3, 0) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(3, 1) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(3, 2) -  0.0000) < epsilon);
	CPPUNIT_ASSERT(fabs(ta.pixel(3, 3) -  0.0000) < epsilon);

	// that's it
	debug(LOG_DEBUG, DEBUG_LOG, 0, "negative translation test complete");
}

} // namespace test
} // namespace astro
