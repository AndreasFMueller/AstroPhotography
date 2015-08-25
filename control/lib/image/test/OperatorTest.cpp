/*
 * OperatorTest.cpp -- test pixel conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroFilter.h>
#include <AstroOperators.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::image::operators;

namespace astro {
namespace test {

class OperatorTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testFlip();

	CPPUNIT_TEST_SUITE(OperatorTest);
	CPPUNIT_TEST(testFlip);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(OperatorTest);

void	OperatorTest::setUp() {
}

void	OperatorTest::tearDown() {
}

void	OperatorTest::testFlip() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFlip() begin");
	Image<unsigned char>	image(10, 10);
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			image.pixel(x, y) = 7 + x + y;
		}
	}
	FlipOperator<unsigned char>	f;
	f(image);
	for (int x = 0; x < image.size().width(); x++) {
		for (int y = 0; y < image.size().height(); y++) {
			CPPUNIT_ASSERT(image.pixel(x, image.size().height() - y - 1)
				== 7 + x + y);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFlip() end");
}

} // namespace test
} // namespace astro
