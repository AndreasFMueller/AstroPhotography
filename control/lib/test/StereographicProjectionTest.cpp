/*
 * StereographicProjectionTest.cpp -- test the stereographic projection
 *
 *  (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::image::transform;

namespace astro {
namespace test {

class StereographicProjectionTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testProjection();

	CPPUNIT_TEST_SUITE(StereographicProjectionTest);
	CPPUNIT_TEST(testProjection);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StereographicProjectionTest);

void	StereographicProjectionTest::setUp() {
}

void	StereographicProjectionTest::tearDown() {
}

void	StereographicProjectionTest::testProjection() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testProjection() begin");
	RaDec	c(M_PI / 4, M_PI / 4);
	StereographicProjection	p(c);
	Point	n = p(RaDec::north_pole);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "stereographic projection of N: %s",
		n.toString().c_str());
	CPPUNIT_ASSERT(fabs(tan(M_PI / 8) - n.y()) < 1e-8);
	CPPUNIT_ASSERT(fabs(n.x()) < 1e-8);

	Point	C = p(c);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "center: %s", C.toString().c_str());
	CPPUNIT_ASSERT(fabs(C.x()) < 1e-8);
	CPPUNIT_ASSERT(fabs(C.y()) < 1e-8);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testProjection() end");
}

} // namespace test
} // namespace astro
