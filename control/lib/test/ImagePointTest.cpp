/*
 * ImagePointTest.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace astro::image;

namespace astro {
namespace test {

class ImagePointTest : public CppUnit::TestFixture {
private:
	ImagePoint	*p1, *p2, *p3;
public:
	void	setUp();
	void	tearDown();
	void	testEquality();
	void	testArithmetic();

	CPPUNIT_TEST_SUITE(ImagePointTest);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testArithmetic);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImagePointTest);

void	ImagePointTest::setUp() {
	p1 = new ImagePoint(3, 5);
	p2 = new ImagePoint(3, 5);
	p3 = new ImagePoint(5, 4);
}

void	ImagePointTest::tearDown() {
	delete p1;
	delete p2;
	delete p3;
}

void	ImagePointTest::testEquality() {
	CPPUNIT_ASSERT(*p1 == *p2);
	CPPUNIT_ASSERT(!(*p2 == *p3));
}

void	ImagePointTest::testArithmetic() {
	CPPUNIT_ASSERT((*p1 + *p3) == ImagePoint(8, 9));
	CPPUNIT_ASSERT((*p1 - *p3) == ImagePoint(-2, 1));
}

} // namespace test
} // namespace astro
