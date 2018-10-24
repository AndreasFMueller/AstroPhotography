/*
 * ParabolicSolverTest.cpp -- test the parabolic solver
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

class ParabolicSolverTest : public CppUnit::TestFixture {
private:
	double	a[3];
	double	p(double x) {
		return (a[2] * x + a[1]) * x + a[0];
	}
	double	frandom() {
		return rand() / (double)RAND_MAX;
	}
public:
	void	setUp();
	void	tearDown();
	void	testBasic();
	void	testRandom();

	CPPUNIT_TEST_SUITE(ParabolicSolverTest);
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST(testRandom);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParabolicSolverTest);

void	ParabolicSolverTest::setUp() {
	a[2] = frandom();
	double	center = 20000 + (frandom() - 0.5) * 10000;
	a[1] = -2 * center * a[2];
	a[0] = center * center * a[2];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "a0 = %.6f, a1 = %.6f, a2 = %.6f",
		a[0], a[1], a[2]);
}

void	ParabolicSolverTest::tearDown() {
}

void	ParabolicSolverTest::testBasic() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() begin");
	FocusItems	focusitems;
	for (int position = 16000; position < 24000; position += 1000) {
		float	value = p(position);
		focusitems.insert(FocusItem(position, value));
	}
	ParabolicSolver	ps;
	int	p = ps.position(focusitems);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "expected: %f, found: %d",
		-a[1] / (2 * a[2]), p);
	CPPUNIT_ASSERT(fabs(p + a[1] / (2 * a[2])) < 50);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() end");
}

void	ParabolicSolverTest::testRandom() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRandom() begin");
	FocusItems	focusitems;
	double	noiselimit = frandom() * p(20000);
	for (int position = 16000; position < 24000; position += 1000) {
		float	value = p(position);
		value += (frandom() - 0.5) * 0.1 * noiselimit;
		focusitems.insert(FocusItem(position, value));
	}
	ParabolicSolver	ps;
	int	p = ps.position(focusitems);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "expected: %f, found: %d",
		-a[1] / (2 * a[2]), p);
	CPPUNIT_ASSERT(fabs(p + a[1] / (2 * a[2])) < 50);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRandom() end");
}

} // namespace test
} // namespace astro
