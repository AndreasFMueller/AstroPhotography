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

using namespace astro::image::transform;
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
	void	testAndromeda();

	CPPUNIT_TEST_SUITE(ProjectionTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testAndromeda);
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
	FITSin	in("testimages/deneb-transform.fits");
	ImagePtr	imageptr = in.read();
	Image<unsigned char>	*image
		= dynamic_cast<Image<unsigned char> *>(&*imageptr);
	CPPUNIT_ASSERT(NULL != image);

	// adapter to convert to double
	TypeReductionAdapter<double, unsigned char>	doubleimage(*image);

	// create the transform
	Projection	projection(-M_PI * 49 / 180, Point(318, 40));

	// apply the transform
	ProjectionAdapter<double>	projected(doubleimage.getSize(),
						doubleimage, projection);

	// create a new image from 
	Image<double>	result(projected);

	// write the image
	FITSoutfile<double>	out("tmp/deneb-projected.fits");
	out.setPrecious(false);
	out.write(result);

	// write the deneb image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	ProjectionTest::testAndromeda() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAndromeda() begin");
	// read the deneb image
	FITSin	in("testimages/andromeda-base.fits");
	ImagePtr	imageptr = in.read();
	Image<unsigned char>	*image
		= dynamic_cast<Image<unsigned char> *>(&*imageptr);
	CPPUNIT_ASSERT(NULL != image);

	// adapter to convert to double
	TypeReductionAdapter<double, unsigned char>	doubleimage(*image);

	// create the transform
	Projection	projection(M_PI * 160 / 180, Point(838, 182));

	// apply the transform
	ProjectionAdapter<double>	projected(doubleimage.getSize(),
						doubleimage, projection);

	// create a new image from 
	Image<double>	result(projected);

	// write the image
	FITSoutfile<double>	out("tmp/andromeda-projected.fits");
	out.setPrecious(false);
	out.write(result);

	// write the deneb image
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAndromeda() end");
}

} // namespace test
} // namespace astro
