/*
 * BinningTests.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroCamera.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>

using namespace astro::camera;

namespace astro {
namespace test {

class BinningTest : public CppUnit::TestFixture {
private:
public:
	void	setUp() { }
	void	tearDown() { }
	void	testEquality();
	void	testPermits();

	CPPUNIT_TEST_SUITE(BinningTest);
	CPPUNIT_TEST(testPermits);
	CPPUNIT_TEST_SUITE_END();
};

void	BinningTest::testPermits() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPermits() begin");
	BinningSet	bs;
	bs.insert(Binning(1,1));
	bs.insert(Binning(2,2));
	bs.insert(Binning(3,3));
	CPPUNIT_ASSERT(bs.permits(Binning(1,1)));
	CPPUNIT_ASSERT(bs.permits(Binning(2,2)));
	CPPUNIT_ASSERT(bs.permits(Binning(3,3)));
	CPPUNIT_ASSERT(!bs.permits(Binning(1,2)));
	CPPUNIT_ASSERT(!bs.permits(Binning(1,3)));
	CPPUNIT_ASSERT(!bs.permits(Binning(2,3)));
	CPPUNIT_ASSERT(!bs.permits(Binning(4,3)));
	CPPUNIT_ASSERT(!bs.permits(Binning(2,1)));
	CPPUNIT_ASSERT(!bs.permits(Binning(3,1)));
	CPPUNIT_ASSERT(!bs.permits(Binning(3,2)));
	CPPUNIT_ASSERT(!bs.permits(Binning(3,4)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPermits() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(BinningTest);

} // namespace test
} // namespace astro
