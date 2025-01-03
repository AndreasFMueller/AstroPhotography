/*
 * AnalyzerTest.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>
#include <AstroIO.h>
#include <AstroDebug.h>
#include <AstroAdapter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::image::transform;
using namespace astro::adapter;
using namespace astro::io;

namespace astro {
namespace test {

class AnalyzerTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testResiduals();

	CPPUNIT_TEST_SUITE(AnalyzerTest);
	CPPUNIT_TEST(testResiduals);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AnalyzerTest);

void	AnalyzerTest::setUp() {
}

void	AnalyzerTest::tearDown() {
}

void	AnalyzerTest::testResiduals() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testResiduals() begin");
	// read the chart image
	FITSinfile<float>	chart("testimages/deneb-chart.fits");
	Image<float>	*image1 = chart.read();
	TypeReductionAdapter<double, float>	base(*image1);

	// read the projected image
	FITSinfile<double>	projected("testimages/deneb-projected.fits");
	Image<double>	*image2 = projected.read();

	// compute the residuals
	Analyzer	analyzer(base);
	std::vector<Residual>	residuals = analyzer(*image2);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d residuals", residuals.size());
	std::vector<Residual>::const_iterator	r;
	for (r = residuals.begin(); r != residuals.end(); r++) {
		// std::cout << r->first << " -> " << r->second << std::endl;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testResiduals() end");
}


} // namespace test
} // namespace astro
