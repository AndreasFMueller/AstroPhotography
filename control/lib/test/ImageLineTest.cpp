/*
 * ImageLineTest.cpp -- tests for the Row/Column access classes
 *
 * (c) 2012 Prof Dr Andreas Mueller,
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

class ImageLineTest : public CppUnit::TestFixture {
private:
	ImageSize	*i;
public:
	void	setUp();
	void	tearDown();
	void	testRow();
	void	testColumn();

	CPPUNIT_TEST_SUITE(ImageLineTest);
	CPPUNIT_TEST(testRow);
	CPPUNIT_TEST(testColumn);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageLineTest);

void	ImageLineTest::setUp() {
	i = new ImageSize(640, 480);
}

void	ImageLineTest::tearDown() {
	delete i;
}

void	ImageLineTest::testColumn() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testColumn() begin");
	ImageLine	*l = new ImageColumn(*i, 47);
	ImageIteratorBase	it = l->begin();
	CPPUNIT_ASSERT(it.pixeloffset() == 47);
	delete l;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testColumn() end");
}

void	ImageLineTest::testRow() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRow() begin");
	ImageLine	*l = new ImageRow(*i, 47);
	ImageIteratorBase	it = l->begin();
	CPPUNIT_ASSERT(it.pixeloffset() == (47 * 640));
	delete l;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRow() end");
}

} // namespace test
} // namespace astro
