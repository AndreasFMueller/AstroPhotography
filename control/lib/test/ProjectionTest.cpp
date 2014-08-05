/*
 * ProjectionTest.cpp -- verify that the projection operator works
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

using namespace astro::image::project;
using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace test {

class ProjectionTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();

	CPPUNIT_TEST_SUITE(ProjectionTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProjectionTest);

void	ProjectionTest::setUp() {
}

void	ProjectionTest::tearDown() {
}

void	ProjectionTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	// read the deneb image
	FITSin	in("deneb-transform.fits");
	ImagePtr	imageptr = in.read();
	Image<unsigned char>	*image
		= dynamic_cast<Image<unsigned char> *>(&*imageptr);
	CPPUNIT_ASSERT(NULL != image);

	// adapter to convert to double
	TypeConversionAdapter<double, unsigned char>	doubleimage(*image);

	// create the transform
	Projection	projection(-M_PI * 49 / 180, Point(318, 40));

	// apply the transform
	ProjectionAdapter<double>	projected(doubleimage.getSize(),
						doubleimage, projection);

	// create a new image from 
	Image<double>	result(projected);

	// write the image
	FITSoutfile<double>	out("deneb-projected.fits");
	out.setPrecious(false);
	out.write(result);

	// write the deneb image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}


} // namespace test
} // namespace astro
