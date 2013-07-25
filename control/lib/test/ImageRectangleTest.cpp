/*
 * ImageRectangleTest.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>

using namespace astro::image;

namespace astro {
namespace test {

class ImageRectangleTest : public CppUnit::TestFixture {
private:
	ImagePoint	*p1, *p2;
	ImageSize	*s1, *s2;
	ImageRectangle	*r1, *r2, *r3, *r4;
public:
	void	setUp();
	void	tearDown();

	void	testAccessors();
	void	testEquality();
	void	testConstructor();
	void	testCorners();
	void	testContainsPoint();
	void	testContainsRectangle();
	void	testTranslation();
	void	testSubrectangle();
	void	testSubrectangleDoesNotFit();

	CPPUNIT_TEST_SUITE(ImageRectangleTest);
	CPPUNIT_TEST(testAccessors);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testContainsPoint);
	CPPUNIT_TEST(testContainsRectangle);
	CPPUNIT_TEST(testCorners);
	CPPUNIT_TEST(testTranslation);
	CPPUNIT_TEST(testSubrectangle);
	CPPUNIT_TEST_EXCEPTION(testSubrectangleDoesNotFit, std::range_error);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageRectangleTest);

void	ImageRectangleTest::setUp() {
	p1 = new ImagePoint(3, 5);
	p2 = new ImagePoint(0, 0);
	s1 = new ImageSize(640, 480);
	s2 = new ImageSize(1024, 768);
	r1 = new ImageRectangle(*p1, *s1);
	r2 = new ImageRectangle(*p1, *s2);
	r3 = new ImageRectangle(*p2, *s1);
	r4 = new ImageRectangle(*p2, *s2);
}

void	ImageRectangleTest::tearDown() {
	delete p1;
	delete p2;
	delete s1;
	delete s2;
	delete r1;
	delete r2;
	delete r3;
	delete r4;
}

void	ImageRectangleTest::testEquality() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEquality() begin");
	CPPUNIT_ASSERT(*r1 == *r1);
	CPPUNIT_ASSERT(*r2 == *r2);
	CPPUNIT_ASSERT(*r3 == *r3);
	CPPUNIT_ASSERT(*r4 == *r4);
	CPPUNIT_ASSERT(!(*r1 == *r2));
	CPPUNIT_ASSERT(!(*r1 == *r3));
	CPPUNIT_ASSERT(!(*r1 == *r4));
	CPPUNIT_ASSERT(!(*r2 == *r3));
	CPPUNIT_ASSERT(!(*r2 == *r4));
	CPPUNIT_ASSERT(!(*r3 == *r4));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEquality() end");
}

void	ImageRectangleTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEquality() begin");
	CPPUNIT_ASSERT(*r3 == ImageRectangle(ImageSize(640, 480)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEquality() end");
}

void	ImageRectangleTest::testAccessors() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccessors() begin");
	CPPUNIT_ASSERT(r1->size == *s1);
	CPPUNIT_ASSERT(r2->size == *s2);
	CPPUNIT_ASSERT(r3->size == *s1);
	CPPUNIT_ASSERT(r4->size == *s2);
	CPPUNIT_ASSERT(r1->origin == *p1);
	CPPUNIT_ASSERT(r2->origin == *p1);
	CPPUNIT_ASSERT(r3->origin == *p2);
	CPPUNIT_ASSERT(r4->origin == *p2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccessors() end");
}

void	ImageRectangleTest::testCorners() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCorners() begin");
	CPPUNIT_ASSERT(ImagePoint(3, 5) == r1->lowerLeftCorner());
	CPPUNIT_ASSERT(ImagePoint(642, 5) == r1->lowerRightCorner());
	CPPUNIT_ASSERT(ImagePoint(3, 484) == r1->upperLeftCorner());
	CPPUNIT_ASSERT(ImagePoint(642, 484) == r1->upperRightCorner());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCorners() end");
}

void	ImageRectangleTest::testContainsPoint() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testContainsPoint() begin");
	CPPUNIT_ASSERT(r1->contains(r1->lowerLeftCorner()));
	CPPUNIT_ASSERT(r1->contains(r1->lowerRightCorner()));
	CPPUNIT_ASSERT(r1->contains(r1->upperLeftCorner()));
	CPPUNIT_ASSERT(r1->contains(r1->upperRightCorner()));
	CPPUNIT_ASSERT(!r1->contains(ImagePoint(2, 5)));
	CPPUNIT_ASSERT(!r1->contains(ImagePoint(3, 4)));
	CPPUNIT_ASSERT(!r1->contains(ImagePoint(643, 5)));
	CPPUNIT_ASSERT(!r1->contains(ImagePoint(642, 4)));
	CPPUNIT_ASSERT(!r1->contains(ImagePoint(2, 484)));
	CPPUNIT_ASSERT(!r1->contains(ImagePoint(3, 485)));
	CPPUNIT_ASSERT(!r1->contains(ImagePoint(643, 484)));
	CPPUNIT_ASSERT(!r1->contains(ImagePoint(642, 485)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testContainsPoint() end");
}

void	ImageRectangleTest::testContainsRectangle() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testContainsRectangle() begin");
	CPPUNIT_ASSERT(r1->contains(*r1));
	CPPUNIT_ASSERT(r2->contains(*r1));
	CPPUNIT_ASSERT(!r1->contains(*r2));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testContainsRectangle() end");
}

void	ImageRectangleTest::testTranslation() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTranslation() begin");
	ImageRectangle	r(*r1, ImagePoint(17, 4));
	CPPUNIT_ASSERT(r.size == r1->size);
	CPPUNIT_ASSERT(r.origin == ImagePoint(20, 9));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTranslation() end");
}

void	ImageRectangleTest::testSubrectangle() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSubrectangle() begin");
	ImageRectangle	r(*r2, *r1);
	CPPUNIT_ASSERT(r.size == r1->size);
	CPPUNIT_ASSERT(r.origin == ImagePoint(6, 10));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSubrectangle() end");
}

void	ImageRectangleTest::testSubrectangleDoesNotFit() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSubrectangleDoesNotFit() begin");
	ImageRectangle	r(*r3, *r2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSubrectangleDoesNotFit() end");
}

} // namespace test
} // namespace astro
