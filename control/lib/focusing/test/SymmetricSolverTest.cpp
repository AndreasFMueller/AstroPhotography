/*
 * SymmetricSolverTest.cpp -- test the parabolic solver
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
#include "../SymmetricSolver.h"

using namespace astro::focusing;

namespace astro {
namespace test {

class SymmetricSolverTest : public CppUnit::TestFixture {
private:
	Function	f;
public:
	void	setUp();
	void	tearDown();
	void	testValues();
	void	testMirror();
	void	testRefine();
	void	testCombine();
	void	testRestrict();
	void	testOperation();

	CPPUNIT_TEST_SUITE(SymmetricSolverTest);
	CPPUNIT_TEST(testValues);
	CPPUNIT_TEST(testMirror);
	CPPUNIT_TEST(testRefine);
	CPPUNIT_TEST(testCombine);
	CPPUNIT_TEST(testRestrict);
	CPPUNIT_TEST(testOperation);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SymmetricSolverTest);

static float	mirrorpoint = 5.2;

static float	f0(float x) {
	return 1. / (1 + powf(fabsf(x - mirrorpoint), 2));
} 

void	SymmetricSolverTest::setUp() {
	if (0 != f.size()) {
		return;
	}
	f.insert(FunctionPoint( 0,f0( 0)));
	f.insert(FunctionPoint( 1,f0( 1)));
	f.insert(FunctionPoint( 3,f0( 3)));
	f.insert(FunctionPoint( 7,f0( 7)));
	f.insert(FunctionPoint( 8,f0( 8)));
	f.insert(FunctionPoint(10,f0(10)));
}

void	SymmetricSolverTest::tearDown() {
}

void	SymmetricSolverTest::testValues() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testValues() begin");
	for (float x = 0; x <= 10; x += 1) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "f(%f) = %f, f0(%f) = %f",
			x, f(x), x, f0(x));
	}
	CPPUNIT_ASSERT(fabs(f( 0) - (       f0(0.0)             )) < 0.00001);
	CPPUNIT_ASSERT(fabs(f( 1) - (       f0(1.0)             )) < 0.00001);
	CPPUNIT_ASSERT(fabs(f( 3) - (       f0(3.0)             )) < 0.00001);
	CPPUNIT_ASSERT(fabs(f( 4) - (0.75 * f0(3) + 0.25 * f0(7))) < 0.00001);
	CPPUNIT_ASSERT(fabs(f( 5) - (0.5  * f0(3) + 0.5  * f0(7))) < 0.00001);
	CPPUNIT_ASSERT(fabs(f( 6) - (0.25 * f0(3) + 0.75 * f0(7))) < 0.00001);
	CPPUNIT_ASSERT(fabs(f( 7) - (       f0(7)               )) < 0.00001);
	CPPUNIT_ASSERT(fabs(f( 8) - (       f0(8)               )) < 0.00001);
	CPPUNIT_ASSERT(fabs(f( 9) - (0.5 * (f0(8) + f0(10))     )) < 0.00001);
	CPPUNIT_ASSERT(fabs(f(10) - (       f0(10)              )) < 0.00001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testValues() end");
}

void	SymmetricSolverTest::testMirror() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMirror() begin");
	Function	m = f.mirror(mirrorpoint);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mirrored: %s", m.toString().c_str());
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 0) - f( 0)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 1) - f( 1)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 2) - f( 2)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 3) - f( 3)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 4) - f( 4)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 5) - f( 5)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 6) - f( 6)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 7) - f( 7)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 8) - f( 8)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint - 9) - f( 9)) < 0.00001);
	CPPUNIT_ASSERT(fabsf(m(mirrorpoint -10) - f(10)) < 0.00001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMirror() end");
}

void	SymmetricSolverTest::testRefine() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRefine() begin");
	Function	g = f;
	for (float x = 0; x < 11; x += 1) {
		g.add(x);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "refined: %s", g.toString().c_str());
	for (int i = 0; i < 11; i++) {
		CPPUNIT_ASSERT(fabsf(g[i] - f( i)) < 0.0001);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRefine() end");
}

void	SymmetricSolverTest::testCombine() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCombine() begin");
	Function	m = f.mirror(mirrorpoint);
	m.add(f);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "combined: %s", m.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCombine() end");
}

void	SymmetricSolverTest::testRestrict() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRestrict() begin");
	Function	m = f.mirror(mirrorpoint);
	Function	r = m.restrict(f);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "restriction: %s", r.toString().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRestrict() end");
}

void	SymmetricSolverTest::testOperation() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOperation() begin");
	Function	d = f.mirror(mirrorpoint) - f;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "restriction: %s", d.toString().c_str());
	float	i = d.integrate();
	float	i2 = d.integrate2();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "integral %f, square-integral %f", i, i2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOperation() end");
}

#if 0
void	SymmetricSolverTest::testCombine() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCombine() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCombine() end");
}
#endif

} // namespace test
} // namespace astro
