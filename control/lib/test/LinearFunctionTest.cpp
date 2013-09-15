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

	CPPUNIT_TEST_SUITE(LinearFunctionTest);
	CPPUNIT_TEST(testBase);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LinearFunctionTest);

void	LinearFunctionTest::setUp() {
}

void	LinearFunctionTest::tearDown() {
}

void	LinearFunctionTest::testBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBase() begin");
	LinearFunctionBase	l;
	l[0] = 0.1;
	l[1] = 0.2;
	l[2] = 0.3;
	std::vector<LinearFunctionBase::doublevaluepair>	values;
	for (unsigned int x = 0; x < 1000; x += 10) {
		for (unsigned int y = 0; y < 1000; y += 10) {
			double	e = -0.5 + random() / (double)2147483647;
			ImagePoint	p(x, y);
			double	value = l.evaluate(Point(p)) + e;
			values.push_back(std::make_pair(p, value));
		}
	}
	LinearFunctionBase	l2(values);
	for (unsigned int x = 0; x < 1000; x += 50) {
		for (unsigned int y = 0; y < 1000; y += 50) {
			ImagePoint	p(x, y);
			double	delta = fabs(l.evaluate(Point(p)) - l2.evaluate(Point(p)));
			CPPUNIT_ASSERT(delta < 2);
		}
	}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBase() end");
}

} // namespace test
} // namespace astro
