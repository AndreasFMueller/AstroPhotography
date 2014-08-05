/*
 * ProjectionAnalyzerTest.cpp
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

using namespace astro::image::project;
using namespace astro::adapter;
using namespace astro::io;

namespace astro {
namespace test {

class ProjectionAnalyzerTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testResiduals();

	CPPUNIT_TEST_SUITE(ProjectionAnalyzerTest);
	CPPUNIT_TEST(testResiduals);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProjectionAnalyzerTest);

void	ProjectionAnalyzerTest::setUp() {
}

void	ProjectionAnalyzerTest::tearDown() {
}

void	ProjectionAnalyzerTest::testResiduals() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testResiduals() begin");
	// read the chart image
	FITSinfile<float>	chart("deneb-chart.fits");
	Image<float>	*image1 = chart.read();
	TypeConversionAdapter<double, float>	base(*image1);

	// read the projected image
	FITSinfile<double>	projected("deneb-projected.fits");
	Image<double>	*image2 = projected.read();

	// compute the residuals
	ProjectionAnalyzer	analyzer(base);
	std::vector<Residual>	residuals = analyzer(*image2);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d residuals", residuals.size());
	std::vector<Residual>::const_iterator	r;
	for (r = residuals.begin(); r != residuals.end(); r++) {
		std::cout << r->first << " -> " << r->second << std::endl;
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testResiduals() end");
}


} // namespace test
} // namespace astro
