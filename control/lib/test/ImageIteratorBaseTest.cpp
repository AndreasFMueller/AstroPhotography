/*
 * ImageIteratorBaseTest.cpp -- tests for the Image Iterator base class
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

class ImageIteratorBaseTest : public CppUnit::TestFixture {
private:
	ImageIteratorBase	*i1;
	ImageIteratorBase	*i2;
	ImageIteratorBase	*i3;
public:
	void	setUp();
	void	tearDown();
	void	testValid();
	void	testIncrement();
	void	testArithmetic();
	void	testPixeloffset();

	CPPUNIT_TEST_SUITE(ImageIteratorBaseTest);
	CPPUNIT_TEST(testValid);
	CPPUNIT_TEST(testIncrement);
	CPPUNIT_TEST(testArithmetic);
	CPPUNIT_TEST(testPixeloffset);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageIteratorBaseTest);

void	ImageIteratorBaseTest::setUp() {
	i1 = new ImageIteratorBase(47, 1247, 1, 200);
	i2 = new ImageIteratorBase(47, 1247, 1247, 200);
	i3 = new ImageIteratorBase(47, 1247, 200);
}

void	ImageIteratorBaseTest::tearDown() {
	delete i1;
	delete i2;
	delete i3;
}

void	ImageIteratorBaseTest::testValid() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testValid() begin");
	CPPUNIT_ASSERT(!i1->valid());
	CPPUNIT_ASSERT(i2->valid());
	CPPUNIT_ASSERT(i3->valid());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testValid() end");
}

void	ImageIteratorBaseTest::testIncrement() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIncrement() begin");
	for (int i = 0; i < 6; i++) {
		(*i3)++;
	}
	CPPUNIT_ASSERT(*i3 == *i2);
	for (int i = 0; i < 6; i++) {
		(*i3)--; --(*i2);
	}
	CPPUNIT_ASSERT(*i3 == *i2);
	for (int i = 0; i < 6; i++) {
		++(*i2);
	}
	ImageIteratorBase	i(*i2);
	CPPUNIT_ASSERT(i-- == *i2);
	CPPUNIT_ASSERT(++i == *i2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIncrement() end");
}

void	ImageIteratorBaseTest::testArithmetic() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testArithmetic() begin");
	CPPUNIT_ASSERT((*i3 + 6) == (*i2));
	CPPUNIT_ASSERT((*i2 - 6) == (*i3));
	(*i1) = *i1 + 7;
	CPPUNIT_ASSERT(!(i1->valid()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testArithmetic() end");
}

void	ImageIteratorBaseTest::testPixeloffset() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPixeloffset() begin");
	ImageIteratorBase	b(7 * 640, 8 * 640 - 1, 7 * 640, 1);
	ImageIteratorBase	e(0, 0, -1, 0);
	ImageIteratorBase	i;
	unsigned int	counter = 0;
	for (i = b; i != e; i++) {
		CPPUNIT_ASSERT(i.pixeloffset() == (7 * 640 + counter));
		counter++;
	}
	CPPUNIT_ASSERT(counter == 640);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPixeloffset() end");
}

} // namespace test
} // namespace astro
