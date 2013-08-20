/*
 * SimUtilTest.cpp -- Test the Simulator utility functions/classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <SimUtil.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <math.h>

namespace astro {
namespace camera {
namespace simulator {
namespace test {

class SimUtilTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testSimTime();

	CPPUNIT_TEST_SUITE(SimUtilTest);
	CPPUNIT_TEST(testSimTime);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SimUtilTest);

void	SimUtilTest::setUp() {
}

void	SimUtilTest::tearDown() {
}

void	SimUtilTest::testSimTime() {
	double	now = simtime();
	simtime_advance(47);
	double	delta = fabs(simtime() - 47 - now);
	CPPUNIT_ASSERT(delta < 0.01);
}

} // namespace test
} // namespace simulator
} // namespace camera
} // namespace astro
