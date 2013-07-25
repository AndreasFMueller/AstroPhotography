/*
 * ImageBaseTests.cpp
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

class ImageBaseTest : public CppUnit::TestFixture {
private:
	ImageBase	*i1;
	ImageBase	*i2;
	ImageBase	*i3;
	ImageBase	*i4;
public:
	void	setUp();
	void	tearDown();
	void	testAccessors();
	void	testEquality();
	void	testPixeloffset();

	CPPUNIT_TEST_SUITE(ImageBaseTest);
	CPPUNIT_TEST(testAccessors);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testPixeloffset);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageBaseTest);

void	ImageBaseTest::setUp() {
	i1 = new ImageBase(ImageSize(640,480));
	i2 = new ImageBase(ImageSize(640,480));
	i3 = new ImageBase(ImageSize(1024,768));
	i4 = new ImageBase(ImageSize(1024,768));
}

void	ImageBaseTest::tearDown() {
	delete i1;
	delete i2;
	delete i3;
	delete i4;
}

void	ImageBaseTest::testAccessors() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccessors() begin");
	CPPUNIT_ASSERT(i1->size == ImageSize(640,480));
	CPPUNIT_ASSERT(i2->size == ImageSize(640,480));
	CPPUNIT_ASSERT(i3->size == ImageSize(1024,768));
	CPPUNIT_ASSERT(i4->size == ImageSize(1024,768));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccessors() end");
}

void	ImageBaseTest::testEquality() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEquality() begin");
	CPPUNIT_ASSERT(*i1 == *i2);
	CPPUNIT_ASSERT(!(*i1 == *i3));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEquality() end");
}

void	ImageBaseTest::testPixeloffset() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPixelOffset() begin");
	CPPUNIT_ASSERT(i1->pixeloffset(4, 11) == (4 + 11 * 640));
	CPPUNIT_ASSERT(i1->pixeloffset(ImagePoint(4, 11)) == (4 + 11 * 640));
	CPPUNIT_ASSERT(i4->pixeloffset(4, 11) == (4 + 1024 * (11)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPixelOffset() end");
}

} // namespace test
} // namespace astro
