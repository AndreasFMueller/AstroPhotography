/*
 * LinearFunctionTests.cpp
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

class LinearFunctionTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testBase();
	void	testSymmetric();
	void	testAsymmetric();

	CPPUNIT_TEST_SUITE(LinearFunctionTest);
	CPPUNIT_TEST(testBase);
	CPPUNIT_TEST(testSymmetric);
	CPPUNIT_TEST(testAsymmetric);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LinearFunctionTest);

void	LinearFunctionTest::setUp() {
}

void	LinearFunctionTest::tearDown() {
}

void	LinearFunctionTest::testBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBase() begin");
	LinearFunction	l(ImagePoint(500, 500), false);
	l[0] = 0.1;
	l[1] = 0.2;
	l[2] = 0.3;
	std::vector<FunctionBase::doublevaluepair>	values;
	for (int x = 0; x < 1000; x += 10) {
		for (int y = 0; y < 1000; y += 10) {
			double	e = -0.5 + random() / (double)2147483647;
			ImagePoint	p(x, y);
			double	value = l.evaluate(Point(p)) + e;
			values.push_back(std::make_pair(p, value));
		}
	}
	LinearFunction	l2(ImagePoint(500, 500), false, values);
	for (int x = 0; x < 1000; x += 50) {
		for (int y = 0; y < 1000; y += 50) {
			ImagePoint	p(x, y);
			double	delta = fabs(l.evaluate(Point(p)) - l2.evaluate(Point(p)));
			CPPUNIT_ASSERT(delta < 2);
		}
	}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBase() end");
}

void	LinearFunctionTest::testSymmetric() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSymmetric() begin");
	ImageSize	size(1000, 1000);
	LinearFunction	l(size.center(), true);
	l[2] = 47;
	std::vector<FunctionBase::doublevaluepair>	values;
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			float	e = -0.5 + random() / (double)2147483647;
			ImagePoint	p(x, y);
			float	value = l(p) + e;
			values.push_back(std::make_pair(p, value));
		}
	}
	ImageFunctionAdapter<LinearFunction>	lfa(size, l, ImagePoint(0, 0));
	MinimumEstimator<LinearFunction>	me(lfa, 100);
	FunctionPtr	l2 = me(size.center(), true);
	
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			ImagePoint	p(x, y);
			double	delta = fabs(l(p) - l2->evaluate(p));
			CPPUNIT_ASSERT(delta < 2);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSymmetric() end");
}

void	LinearFunctionTest::testAsymmetric() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAsymmetric() begin");
	ImageSize	size(1000, 1000);
	LinearFunction	l(size.center(), false);
	l[0] = 0.1;
	l[1] = 0.2;
	l[2] = 200;
	std::vector<FunctionBase::doublevaluepair>	values;
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			float	e = -0.5 + random() / (double)2147483647;
			ImagePoint	p(x, y);
			float	value = l(p) + e;
			values.push_back(std::make_pair(p, value));
		}
	}
	ImageFunctionAdapter<LinearFunction>	lfa(size, l, ImagePoint(0, 0));
	MinimumEstimator<LinearFunction>	me(lfa, 100);
	FunctionPtr	l2 = me(size.center(), false);
	
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			ImagePoint	p(x, y);
			double	delta = fabs(l(p) - l2->evaluate(p));
			CPPUNIT_ASSERT(delta < 2);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAsymmetric() end");
}

} // namespace test
} // namespace astro
