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
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testPermits);
	CPPUNIT_TEST_SUITE_END();
};

void	BinningTest::testEquality() {
	Binning	b;
	CPPUNIT_ASSERT(Binning(1, 1).compatible(b));
	CPPUNIT_ASSERT(!(Binning(1, 1).compatible(Binning(-1, 2))));
	CPPUNIT_ASSERT(!(Binning(1, 1).compatible(Binning(2, -1))));
	CPPUNIT_ASSERT(!(Binning(-1, 2).compatible(Binning(1, 1))));
	CPPUNIT_ASSERT(!(Binning(2, -1).compatible(Binning(1, 1))));
	CPPUNIT_ASSERT(!(Binning(2, 20).compatible(Binning(2, 21))));
}

void	BinningTest::testPermits() {
	BinningSet	bs;
	bs.push_back(Binning(1,1));
	bs.push_back(Binning(2,2));
	bs.push_back(Binning(3,3));
	bs.push_back(Binning(-1,1));
	bs.push_back(Binning(-1,2));
	bs.push_back(Binning(1,-1));
	bs.push_back(Binning(2,-1));
	CPPUNIT_ASSERT(bs.permits(Binning(1,1)));
	CPPUNIT_ASSERT(bs.permits(Binning(2,2)));
	CPPUNIT_ASSERT(bs.permits(Binning(3,3)));
	CPPUNIT_ASSERT(bs.permits(Binning(1,2)));
	CPPUNIT_ASSERT(bs.permits(Binning(1,3)));
	CPPUNIT_ASSERT(bs.permits(Binning(2,3)));
	CPPUNIT_ASSERT(!bs.permits(Binning(4,3)));
	CPPUNIT_ASSERT(bs.permits(Binning(2,1)));
	CPPUNIT_ASSERT(bs.permits(Binning(3,1)));
	CPPUNIT_ASSERT(bs.permits(Binning(3,2)));
	CPPUNIT_ASSERT(!bs.permits(Binning(3,4)));
}

CPPUNIT_TEST_SUITE_REGISTRATION(BinningTest);

} // namespace test
} // namespace astro
