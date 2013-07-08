/*
 * ImageSizeTest.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <debug.h>

using namespace astro::image;

namespace astro {
namespace test {

class ImageSizeTest : public CppUnit::TestFixture {
private:
	ImageSize	*i1, *i2, *i3;
public:
	void	setUp();
	void	tearDown();

	void	testEquality();
	void	testPixels();
	void	testBounds();

	CPPUNIT_TEST_SUITE(ImageSizeTest);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testPixels);
	CPPUNIT_TEST(testBounds);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageSizeTest);

void	ImageSizeTest::setUp() {
	i1 = new ImageSize(7, 11);
	i2 = new ImageSize(3, 5);
	i3 = new ImageSize(11, 7);
}

void	ImageSizeTest::tearDown() {
	delete i1;
	delete i2;
}

void	ImageSizeTest::testEquality() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEquality() begin");
	CPPUNIT_ASSERT(*i1 == *i1);
	CPPUNIT_ASSERT(!(*i1 == *i3));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEquality() end");
}

void	ImageSizeTest::testPixels() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPixels() begin");
	CPPUNIT_ASSERT(i1->pixels == 77);
	CPPUNIT_ASSERT(i2->pixels == 15);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPixels() end");
}

void	ImageSizeTest::testBounds() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBounds() begin");
	CPPUNIT_ASSERT(i1->bounds(ImagePoint(0, 0)));
	CPPUNIT_ASSERT(i1->bounds(ImagePoint(6, 0)));
	CPPUNIT_ASSERT(i1->bounds(ImagePoint(0, 10)));
	CPPUNIT_ASSERT(i1->bounds(ImagePoint(6, 10)));
	CPPUNIT_ASSERT(!i1->bounds(ImagePoint(0, -1)));
	CPPUNIT_ASSERT(!i1->bounds(ImagePoint(6, -1)));
	CPPUNIT_ASSERT(!i1->bounds(ImagePoint(0, 11)));
	CPPUNIT_ASSERT(!i1->bounds(ImagePoint(6, 11)));
	CPPUNIT_ASSERT(!i1->bounds(ImagePoint(-1, 0)));
	CPPUNIT_ASSERT(!i1->bounds(ImagePoint(-1, 10)));
	CPPUNIT_ASSERT(!i1->bounds(ImagePoint(7, 0)));
	CPPUNIT_ASSERT(!i1->bounds(ImagePoint(7, 10)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBounds() end");
}

} // namespace test
} // namespace astro
