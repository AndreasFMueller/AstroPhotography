/*
 * DebayerTest.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroIO.h>
#include <AstroDebug.h>
#include <AstroDemosaicAdapter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::adapter::demosaic;
using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace test {

class DebayerTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testDebayer();

	CPPUNIT_TEST_SUITE(DebayerTest);
	CPPUNIT_TEST(testDebayer);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DebayerTest);

void	DebayerTest::setUp() {
}

void	DebayerTest::tearDown() {
}

void	DebayerTest::testDebayer() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDebayer() begin");
	// read the input image
	FITSinfile<unsigned short>	in("testimages/debayer.fits");
	Image<unsigned short>	*bayerimage = in.read();

	// create the adapter
	{
		DemosaicAdapter<unsigned short>	demosaicer(*bayerimage,
			MosaicType("RGGB"));

		// actually demosaic the image
		Image<RGB<unsigned short> >	*colorimage
			= new Image<RGB<unsigned short> >(demosaicer);
		FITSoutfile<RGB<unsigned short> >	out(
			"testimages/debayered-rggb.fits");
		out.setPrecious(false);
		out.write(*colorimage);
	}
	{
		DemosaicAdapter<unsigned short>	demosaicer(*bayerimage,
			MosaicType("GRBG"));

		// actually demosaic the image
		Image<RGB<unsigned short> >	*colorimage
			= new Image<RGB<unsigned short> >(demosaicer);
		FITSoutfile<RGB<unsigned short> >	out(
			"testimages/debayered-grbg.fits");
		out.setPrecious(false);
		out.write(*colorimage);
	}
	{
		DemosaicAdapter<unsigned short>	demosaicer(*bayerimage,
			MosaicType("GBRG"));

		// actually demosaic the image
		Image<RGB<unsigned short> >	*colorimage
			= new Image<RGB<unsigned short> >(demosaicer);
		FITSoutfile<RGB<unsigned short> >	out(
			"testimages/debayered-gbrg.fits");
		out.setPrecious(false);
		out.write(*colorimage);
	}
	{
		DemosaicAdapter<unsigned short>	demosaicer(*bayerimage,
			MosaicType("BGGR"));

		// actually demosaic the image
		Image<RGB<unsigned short> >	*colorimage
			= new Image<RGB<unsigned short> >(demosaicer);
		FITSoutfile<RGB<unsigned short> >	out(
			"testimages/debayered-bggr.fits");
		out.setPrecious(false);
		out.write(*colorimage);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDebayer() end");
}

} // namespace test
} // namespace astro
