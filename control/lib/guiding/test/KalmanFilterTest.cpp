/*
 * KalmanFilterTest.cpp -- test of the backlash analysis test
 *
 * (c) 2017 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroGuiding.h>
#include "../KalmanFilter.h"
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <iostream>
#include <includes.h>
#include <cmath>

using namespace astro::guiding;

namespace astro {
namespace test {

class KalmanFilterTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testFilter();

	CPPUNIT_TEST_SUITE(KalmanFilterTest);
	CPPUNIT_TEST(testFilter);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(KalmanFilterTest);

void	KalmanFilterTest::setUp() {
}

void	KalmanFilterTest::tearDown() {
}


static double	r() {
	return ((double)random()) / 2147483647 - 0.5;
}

void	KalmanFilterTest::testFilter() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start KalmanFilter test");

	KalmanFilter	kf(1);
	kf.measurementerror(1);
	kf.systemerror(1);

	std::ofstream	out("test.csv");
	out << "      ox,      oy,        nx,      ny,        ex,      ey,      x[0],    x[1],    x[2],    x[3]" << std::endl;

	for (int i = 0; i < 400; i++) {
		double	angle = M_PI * (10. * i / 180.);
		Point	offset(cos(angle), sin(angle));
		offset = offset * 10.;
		Point	noisy = Point(r(), r()) + offset;
		kf.update(noisy);
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"offset %s, update %s, filtered %s",
			offset.toString().c_str(),
			noisy.toString().c_str(),
			kf.offset().toString().c_str()); 
		out << stringprintf("%8.3f,%8.3f,", offset.x(), offset.y());
		out << stringprintf("%10.3f,%8.3f,", noisy.x(), noisy.y());
		out << stringprintf("%10.3f,%8.3f,", kf.offset().x(), kf.offset().y());
		astro::guiding::Vector<double,4>	v;
		v = kf.state();
		out << stringprintf("%10.3f,%8.3f,%8.3f,%8.3f", v[0], v[1], v[2], v[3]);
		out << std::endl;
	}
	out.close();

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end KalmanFilter test");
}

} // namespace test
} // namespace astro
