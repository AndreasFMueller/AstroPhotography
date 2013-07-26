/*
 * PhaseCorrelatorTest.cpp -- test adapters to subwindows
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <iostream>

using namespace astro::image;
using namespace astro::image::transform;

namespace astro {
namespace test {

class PhaseCorrelatorTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testInteger();
	void	testIntegerNegative();
	void	testHalf();

	CPPUNIT_TEST_SUITE(PhaseCorrelatorTest);
	CPPUNIT_TEST(testInteger);
	CPPUNIT_TEST(testIntegerNegative);
	CPPUNIT_TEST(testHalf);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhaseCorrelatorTest);

void	PhaseCorrelatorTest::setUp() {
}

void	PhaseCorrelatorTest::tearDown() {
}

void	PhaseCorrelatorTest::testInteger() {
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (unsigned int x = 0; x < fromimage.size.getWidth(); x++) {
		for (unsigned int y = 0; y < fromimage.size.getHeight(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * cos(r / 10) / r;
			}
			r = hypot(x - 63, y - 74);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * cos(r / 10) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage);
	std::cout << translation << std::endl;
}

void	PhaseCorrelatorTest::testIntegerNegative() {
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (unsigned int x = 0; x < fromimage.size.getWidth(); x++) {
		for (unsigned int y = 0; y < fromimage.size.getHeight(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * cos(r / 10) / r;
			}
			r = hypot(x - 57, y - 66);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * cos(r / 10) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage);
	std::cout << translation << std::endl;
}

void	PhaseCorrelatorTest::testHalf() {
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (unsigned int x = 0; x < fromimage.size.getWidth(); x++) {
		for (unsigned int y = 0; y < fromimage.size.getHeight(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * cos(r / 8) / r;
			}
			r = hypot(x - 62.5, y - 66.5);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * cos(r / 8) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage);
	std::cout << translation << std::endl;
}


} // namespace test
} // namespace astro
