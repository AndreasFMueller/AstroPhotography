/*
 * BacklashAnalysisTest.cpp -- test of the backlash analysis test
 *
 * (c) 2017 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include <Backlash.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <iostream>
#include <includes.h>

using namespace astro::guiding;

namespace astro {
namespace test {

class BacklashAnalysisTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testAnalysis();

	CPPUNIT_TEST_SUITE(BacklashAnalysisTest);
	CPPUNIT_TEST(testAnalysis);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BacklashAnalysisTest);

void	BacklashAnalysisTest::setUp() {
}

void	BacklashAnalysisTest::tearDown() {
}

void	BacklashAnalysisTest::testAnalysis() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start BacklashAnalysis test");

	double	bplus = 0.7;
	double	dplus = 1.0;
	double	bminus = -0.6;
	double	dminus = -1.2;

	double	a0 = 47;
	double	a1 = 0.1;

	debug(LOG_DEBUG, DEBUG_LOG, 0, "%6.2f %6.2f %6.2f %6.2f %6.2f %6.2f",
		bplus, dplus, bminus, dminus, a0, a1);

	int	k[4] = { 0, 0, 0, 0 };

	std::vector<BacklashPoint>	points;
	int	counter = 0;
	int	t = 0;
	int	N = 40;
	double	time = 0;
	while (t < N) {
		double	x = k[0] * bplus + k[1] * dplus
			+ k[2] * bminus + k[3] * dminus
			+ a0 + a1 * time;
		k[t % 4]++;
		BacklashPoint	p;
		p.id = counter++;
		time += 3 + random() / (double)RAND_MAX;
		p.xoffset = x;
		p.yoffset = 0.2 * random() / (double)RAND_MAX;
		p.time = time;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "x[%d] = %f, time = %f", t, x, time);
		points.push_back(p);
		t++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "have %d points", points.size());
	CPPUNIT_ASSERT(N = points.size());

	BacklashAnalysis	analysis(backlash_dec);
	BacklashResult	result = analysis(points);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end BacklashAnalysis test");
}

} // namespace test
} // namespace astro
