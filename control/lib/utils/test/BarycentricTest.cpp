/*
 * BarycentricTest.cpp -- Test the stacktrace handler
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroCoordinates.h>

namespace astro {
namespace test {

class BarycentricTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testBarycentric();

	CPPUNIT_TEST_SUITE(BarycentricTest);
	CPPUNIT_TEST(testBarycentric);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BarycentricTest);

void	BarycentricTest::setUp() {
}

void	BarycentricTest::tearDown() {
}

void	BarycentricTest::testBarycentric() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBarycentric() begin");
	Point	p1(1,1);
	Point	p2(5,1);
	Point	p3(3,4);

	BarycentricCoordinates	bc(p1, p2, p3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "bc = %s", bc.toString().c_str());

	BarycentricPoint	b1 = bc(p1);
	double	d = (b1 - Point(1,0)).abs();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s, d=%f", p1.toString().c_str(),
		b1.toString().c_str(), d);
	CPPUNIT_ASSERT(d < 0.1);

	BarycentricPoint	b2 = bc(p2);
	d = (b2 - Point(0,1)).abs();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s, d=%f", p2.toString().c_str(),
		b2.toString().c_str(), d);
	CPPUNIT_ASSERT(d < 0.1);

	BarycentricPoint	b3 = bc(p3);
	d = (b3 - Point(0,0)).abs();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s, d=%f", p3.toString().c_str(),
		b3.toString().c_str(), d);
	CPPUNIT_ASSERT(d < 0.1);

	BarycentricPoint	s(1./3, 1./3, 1./3);
	Point	S = bc(s);
	d = (S - Point(3,2)).abs();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s, d=%f",
		s.toString().c_str(), S.toString().c_str(), d);
	CPPUNIT_ASSERT(d < 1e-10);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBarycentric() end");
}

} // namespace test
} // namespace astro
