/*
 * QuadraticFunctionTests.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroBackground.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace test {

class QuadraticFunctionTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testSymmetric();
	void	testAsymmetric();

	CPPUNIT_TEST_SUITE(QuadraticFunctionTest);
	CPPUNIT_TEST(testSymmetric);
	CPPUNIT_TEST(testAsymmetric);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(QuadraticFunctionTest);

void	QuadraticFunctionTest::setUp() {
}

void	QuadraticFunctionTest::tearDown() {
}

void	QuadraticFunctionTest::testSymmetric() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSymmetric() begin");
	ImageSize	size(1000, 1000);
	QuadraticFunction	l(size.center(), true);
	l[2] = 47;
	l[3] = 0.001;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic function: %s",
		l.toString().c_str());
	std::vector<FunctionBase::doublevaluepair>	values;
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			float	e = -0.5 + random() / (double)2147483647;
			ImagePoint	p(x, y);
			float	value = l(p) + e;
			values.push_back(std::make_pair(p, value));
		}
	}
	ImageFunctionAdapter<QuadraticFunction>	lfa(size, l, ImagePoint(0, 0));
	MinimumEstimator<QuadraticFunction>	me(lfa, 100);
	FunctionPtr	l2 = me(size.center(), true);
	
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			ImagePoint	p(x, y);
			double	delta = fabs(l(p) - l2->evaluate(p));
			CPPUNIT_ASSERT(delta < 2);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSymmetric() end");
}

void	QuadraticFunctionTest::testAsymmetric() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAsymmetric() begin");
	ImageSize	size(1000, 1000);
	QuadraticFunction	l(size.center(), false);
	l[0] = 0.1;
	l[1] = 0.2;
	l[2] = 1000;
	l[3] = 0.001;
	l[4] = 0.002;
	l[5] = 0.003;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "quadratic function to find: %s",
		l.toString().c_str());
	std::vector<FunctionBase::doublevaluepair>	values;
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			float	e = -0.5 + random() / (double)2147483647;
			ImagePoint	p(x, y);
			float	value = l(p) + e;
			values.push_back(std::make_pair(p, value));
		}
	}
	ImageFunctionAdapter<QuadraticFunction>	lfa(size, l, ImagePoint(0, 0));
	MinimumEstimator<QuadraticFunction>	me(lfa, 100);
	FunctionPtr	l2 = me(size.center(), false);
	
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			ImagePoint	p(x, y);
			double	delta = fabs(l(p) - l2->evaluate(p));
			CPPUNIT_ASSERT(delta < 2);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAsymmetric() end");
}

} // namespace test
} // namespace astro
