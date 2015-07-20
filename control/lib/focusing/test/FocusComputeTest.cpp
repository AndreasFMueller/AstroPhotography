/*
 * FocusComputeTest.cpp -- test pixel conversions
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <math.h>
#include <AstroDebug.h>
#include <FocusCompute.h>

using namespace astro::focusing;

namespace astro {
namespace test {

class FocusComputeTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testMin();
	void	test10();
	void	testrandom();

	CPPUNIT_TEST_SUITE(FocusComputeTest);
	CPPUNIT_TEST(testMin);
	CPPUNIT_TEST(test10);
	CPPUNIT_TEST(testrandom);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FocusComputeTest);

void	FocusComputeTest::setUp() {
}

void	FocusComputeTest::tearDown() {
}

void	FocusComputeTest::testMin() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMin() begin");
	FocusCompute	focuscompute;
	focuscompute.insert(FocusCompute::value_type(1000, 10.));
	focuscompute.insert(FocusCompute::value_type(2000, 10.));
	focuscompute.insert(FocusCompute::value_type(3000, 30.));
	double	f = focuscompute.focus();
	CPPUNIT_ASSERT(fabs(f - 1500.) < 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMin() end");
}

void	FocusComputeTest::test10() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test10() begin");
	FocusCompute	focuscompute;
	focuscompute.insert(FocusCompute::value_type(1000, 22.));
	focuscompute.insert(FocusCompute::value_type(2000, 17.));
	focuscompute.insert(FocusCompute::value_type(3000, 12.));
	focuscompute.insert(FocusCompute::value_type(4000,  7.));
	focuscompute.insert(FocusCompute::value_type(5000,  2.));
	focuscompute.insert(FocusCompute::value_type(6000,  3.));
	focuscompute.insert(FocusCompute::value_type(7000,  8.));
	focuscompute.insert(FocusCompute::value_type(8000, 13.));
	focuscompute.insert(FocusCompute::value_type(9000, 18.));
	focuscompute.insert(FocusCompute::value_type(10000, 23.));
	double	f = focuscompute.focus();
	CPPUNIT_ASSERT(fabs(f - 5400.) < 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test10() end");
}

double	randomerror() {
	return (random() / 1073741824.) - 1.;
}

void	FocusComputeTest::testrandom() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test10() begin");
	FocusCompute	focuscompute;
	focuscompute.insert(FocusCompute::value_type(1000, 220. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(2000, 170. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(3000, 120. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(4000,  70. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(5000,  20. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(6000,  30. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(7000,  80. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(8000, 130. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(9000, 180. + randomerror()));
	focuscompute.insert(FocusCompute::value_type(10000, 230. + randomerror()));
	double	f = focuscompute.focus();
	CPPUNIT_ASSERT(fabs(f - 5400.) < 10);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test10() end");
}


} // namespace test
} // namespace astro
