/*
 * LinearRegressionTest.cpp -- test of the backlash analysis test
 *
 * (c) 2017 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include "../LinearRegression.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <iostream>
#include <includes.h>

using namespace astro::linear;

namespace astro {
namespace test {

class LinearRegressionTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testLR();

	CPPUNIT_TEST_SUITE(LinearRegressionTest);
	CPPUNIT_TEST(testLR);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LinearRegressionTest);

void	LinearRegressionTest::setUp() {
}

void	LinearRegressionTest::tearDown() {
}

void	LinearRegressionTest::testLR() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start LinearRegression test");

	double	a = 1;
	double	b = 2;
	int	N = 40;

	std::vector<std::pair<double, double> >	data;
	for (int x = 0; x < N; x++) {
		double	X = x + 0.1 * random() / (double)RAND_MAX;
		double	Y = a * x + b + 0.1 * random() / (double)RAND_MAX;
		data.push_back(std::make_pair(X, Y));
	}
	LinearRegression	lr(data);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "a = %f, b = %f", a, b);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end LinearRegression test");
}

} // namespace test
} // namespace astro
