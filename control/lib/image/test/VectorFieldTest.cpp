/*
 * VectorFieldTest.cpp -- test vector field methods
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <cstdlib>
#include <iostream>

using namespace astro::image::transform;
using namespace astro::image;

namespace astro {
namespace test {

class VectorFieldTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testVerify();
	void	testRandom();
	void	testEliminate();
	//void	testXXX();

        CPPUNIT_TEST_SUITE(VectorFieldTest);
//	CPPUNIT_TEST(testVerify);
//	CPPUNIT_TEST(testRandom);
	CPPUNIT_TEST(testEliminate);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VectorFieldTest);

void	VectorFieldTest::setUp() {
}

void	VectorFieldTest::tearDown() {
}

static double	disturb(double r) {
	return random() * r / RAND_MAX;
}

static std::vector<std::pair<ImagePoint, Point> >	grid(double angle, double r) {
	Transform	t(angle, Point(15,27));
	std::vector<std::pair<ImagePoint, Point> >	l;
	for (int x = 256; x < 4096; x += 512) {
		for (int y = 256; y < 3000; y += 512) {
			ImagePoint	im(x, y);
			Point	p0(x, y);
			Point	p = t(p0) - p0 + Point(disturb(r), disturb(r));
			l.push_back(std::make_pair(im, p));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "generated grid with %lu points",
		l.size());
	return l;
}

void	VectorFieldTest::testVerify() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testVerify() begin");
	std::vector<std::pair<ImagePoint, Point> >	l = grid(0.01, 1);
	VectorField	vf(l);
	int	d = vf.verify(0.11);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d points evicted", d);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testVerify() end");
}

void	VectorFieldTest::testRandom() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRandom() begin");
	std::vector<std::pair<ImagePoint, Point> >	l = grid(0.01, 10);
	VectorField	vf(l);
	int	d = vf.verify(0.0115);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d points evicted, %lu remaining",
		d, l.size());
	std::vector<std::pair<ImagePoint, Point> >::const_iterator	i;
	for (i = vf.begin(); i != vf.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
			i->first.toString().c_str(),
			i->second.toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRandom() end");
}

void	VectorFieldTest::testEliminate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEliminate() begin");
	std::vector<std::pair<ImagePoint, Point> >	l = grid(0.01, 10);
	VectorField	vf(l);
	double	t = vf.eliminate(10);
	VectorField::fielddata_t	r = vf.badpoints(t);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "tol=%f eliminates %lu points",
		t, r.size());
	VectorField::fielddata_t::const_iterator        i;
	for (i = r.begin(); i != r.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
			i->first.toString().c_str(),
			i->second.toString().c_str());
	}
	t = vf.eliminate(5);
	r = vf.badpoints(t);
	for (i = r.begin(); i != r.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s -> %s",
			i->first.toString().c_str(),
			i->second.toString().c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEliminate() end");
}

#if 0
void	VectorFieldTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
