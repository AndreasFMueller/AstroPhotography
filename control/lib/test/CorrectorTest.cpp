/*
 * CorrectorTest.cpp -- verify that the projection operator works
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProjection.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroAdapter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::image::transform;
using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace test {

class CorrectorTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testCorrector();
	void	testAndromeda();

	CPPUNIT_TEST_SUITE(CorrectorTest);
	CPPUNIT_TEST(testCorrector);
	CPPUNIT_TEST(testAndromeda);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CorrectorTest);

void	CorrectorTest::setUp() {
}

void	CorrectorTest::tearDown() {
}

void	CorrectorTest::testCorrector() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCorrector() begin");
#if 0

	// read the deneb image
	FITSin	in("deneb-transform.fits");
	ImagePtr	imageptr = in.read();
	Image<unsigned char>	*image
		= dynamic_cast<Image<unsigned char> *>(&*imageptr);
	CPPUNIT_ASSERT(NULL != image);

	// adapter to convert to double
	TypeConversionAdapter<double, unsigned char>	doubleimage(*image);

	// read the chart image
	FITSinfile<float>	chart("deneb-chart.fits");
	Image<float>	*image1 = chart.read();
	TypeConversionAdapter<double, float>	base(*image1);

	// compute the residuals
	Analyzer	analyzer(base);

	// create the transform
	Projection	projection(-M_PI * 49 / 180, Point(318, 40));

	// now iterate corrections
	for (int i = 0; i < 10; i++) {
		// apply the transform
		ProjectionAdapter<double>	projected(doubleimage.getSize(),
							doubleimage, projection);

		// create a new image from 
		Image<double>	result(projected);

		std::vector<Residual>	residuals = analyzer(result);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d residuals", residuals.size());

		// use the residuals to compute the correction
		ProjectionCorrector	corrector(base.getSize(), image->size(),
						projection);
		projection = corrector.corrected(residuals);
	}

	// apply the transform
	ProjectionAdapter<double>	projected(doubleimage.getSize(),
						doubleimage, projection);

	// create a new image from 
	Image<double>	result(projected);

	// write the final corrected image 
	FITSoutfile<double>	out("deneb-corrected.fits");
	out.setPrecious(false);
	out.write(result);
#endif
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCorrector() end");
}

void	CorrectorTest::testAndromeda() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAndromeda() begin");

	// read the deneb image
	FITSin	in("andromeda-base.fits");
	ImagePtr	imageptr = in.read();
	Image<unsigned char>	*image
		= dynamic_cast<Image<unsigned char> *>(&*imageptr);
	CPPUNIT_ASSERT(NULL != image);

	// adapter to convert to double
	TypeConversionAdapter<double, unsigned char>	doubleimage(*image);

	// read the chart image
	FITSinfile<float>	chart("andromeda-chart.fits");
	Image<float>	*image1 = chart.read();
	TypeConversionAdapter<double, float>	base(*image1);

	// compute the residuals
	Analyzer	analyzer(base, 64, 64);

	// create the transform
	Projection	projection(M_PI * 160 / 180, Point(838, 182), 0.98);

	// now iterate corrections
	for (int i = 0; i < 500; i++) {
		// apply the transform
		ProjectionAdapter<double>	projected(doubleimage.getSize(),
							doubleimage, projection);

		// create a new image from 
		Image<double>	result(projected);

		std::vector<Residual>	residuals = analyzer(result);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d residuals", residuals.size());

		// use the residuals to compute the correction
		ProjectionCorrector	corrector(base.getSize(), image->size(),
						projection);
		projection = corrector.corrected(residuals);

		// write the final corrected image 
		FITSoutfile<double>	out(stringprintf("andromeda-corrected-%d.fits", i));
		out.setPrecious(false);
		out.write(result);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAndromeda() end");
}


} // namespace test
} // namespace astro
