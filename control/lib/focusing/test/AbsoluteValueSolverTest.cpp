/*
 * AbsoluteValueSolverTest.cpp -- test the parabolic solver
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <math.h>
#include <AstroDebug.h>
#include <AstroFocus.h>
#include <cstdlib>
#include "../FocusSolvers.h"

using namespace astro::focusing;

namespace astro {
namespace test {

class AbsoluteValueSolverTest : public CppUnit::TestFixture {
private:
	double	a;
	double	center;
	double	p(double x) {
		return a * fabs(x - center);
	}
	double	frandom() {
		return rand() / (double)RAND_MAX;
	}
public:
	void	setUp();
	void	tearDown();
	void	testBasic();
	void	testRandom();

	CPPUNIT_TEST_SUITE(AbsoluteValueSolverTest);
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST(testRandom);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AbsoluteValueSolverTest);

void	AbsoluteValueSolverTest::setUp() {
	a = frandom();
	center = 20000 + 0.5 * (frandom() - 0.5) * 10000;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "a = %f, center = %f", a, center);
}

void	AbsoluteValueSolverTest::tearDown() {
}

void	AbsoluteValueSolverTest::testBasic() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() begin");
	FocusItems	focusitems;
	for (int position = 16000; position < 24000; position += 1000) {
		float	value = p(position);
		focusitems.insert(FocusItem(position, value));
	}
	AbsoluteValueSolver	avs;
	int	p = avs.position(focusitems);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "expected: %f, found: %d", center, p);
	CPPUNIT_ASSERT(fabs(p - center) < 50);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() end");
}

void	AbsoluteValueSolverTest::testRandom() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRandom() begin");
	FocusItems	focusitems;
	double	noiselimit = p(20000);
	for (int position = 16000; position < 24000; position += 1000) {
		float	value = p(position);
		value += (frandom() - 0.5) * 0.1 * noiselimit;
		focusitems.insert(FocusItem(position, value));
	}
	AbsoluteValueSolver	avs;
	int	p = avs.position(focusitems);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "expected: %f, found: %d", center, p);
	CPPUNIT_ASSERT(fabs(p - center) < 50);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRandom() end");
}

} // namespace test
} // namespace astro
