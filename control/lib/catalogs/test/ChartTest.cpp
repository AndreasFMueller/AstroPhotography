/*
 * ChartTest.cpp -- tests for the Chart class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroChart.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroIO.h>

using namespace astro::catalog;
using namespace astro::image;

namespace astro {
namespace test {

class ChartTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testImage();

	CPPUNIT_TEST_SUITE(ChartTest);
	CPPUNIT_TEST(testImage);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ChartTest);

void	ChartTest::setUp() {
}

void	ChartTest::tearDown() {
}

#define CENTER_ANDROMEDA	0
#define	CENTER_ORION		1
#define CENTER_M13		2
#define CENTER_SMC		3
#define CENTER_DENEB		4
#define CENTER_36UMA		5
#define CENTER_M31		6

static int	centerpoint = CENTER_M13;

#define CAMERA_SXMC26C_50MM	0
#define CAMERA_SXMC26C_135MM	1
#define CAMERA_SXMC26C_560MM	2
#define CAMERA_SBIG_2800MM	3

static int	camera = CAMERA_SXMC26C_560MM;

#define PSF_TURBULENCE	0
#define PSF_DIFFRACTION	1
#define PSF_CIRCLE	2

static int	psfchoice = PSF_TURBULENCE;

void	ChartTest::testImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImage() begin");
	
	double	scale = 1;
	double	limit_mag = 14;

	// get the center, in the constellation andromeda
	RaDec	center;
	switch (centerpoint) {
	case CENTER_ANDROMEDA:
		// get the center, in the constellation andromeda
		center.ra().hours(42./60 + 44./3600);
		center.dec().degrees(41 + 16./60 + 10./3600);
		break;
	case CENTER_ORION:
		// Orion
		center.ra().hours(5 + 36/60. + 12.8/3600.);
		center.dec().degrees(-1 - 12./60 -6.9/3600);
		break;
	case CENTER_M13:
		// M13
		center.ra().hours(16 + 41./60 + 41.44/3600);
		center.dec().degrees(36 + 27./60 + 36.9/3600);
		scale = 0.001;
		break;
	case CENTER_SMC:
		// small magellanic cloud
		center.ra().hours(0 + 51./60);
		center.dec().degrees(-73 - 6./60);
		break;
	case CENTER_DENEB:
		// Deneb
		center.ra().hours(20. + 41./60 + 25.9/3600);
		center.dec().degrees(45 + 16./60 + 49./3600);
		break;
	case CENTER_36UMA:
		//  36UMa
		center.ra().hours(10. + 30./60 + 37.6/3600);
		center.dec().degrees(55 + 58./60 + 50.0/3600);
		break;
	case CENTER_M31:
		// M31
		center.ra().hours(0. + 42./60 + 44.3/3600);
		center.dec().degrees(41 + 16./60 + 9./3600);
		break;
	}

	// create chart object
	debug(LOG_DEBUG, DEBUG_LOG, 0, "create the geometry");
	ImageGeometry	geometry;
	switch (camera) {
	case CAMERA_SXMC26C_50MM:
		// SX MC26C 50mm 
		geometry = ImageGeometry(ImageSize(3900, 2616),
			0.050, 0.00000605);
		geometry.aperture(0.050 / 1.9);
		break;
	case CAMERA_SXMC26C_135MM:
		// SX MC26C 135mm 
		geometry = ImageGeometry(ImageSize(3900, 2616),
			0.135, 0.00000605);
		geometry.aperture(0.125 / 2.8);
		break;
	case CAMERA_SXMC26C_560MM:
		// SX MC26, primary focus
		geometry = ImageGeometry(ImageSize(3900, 2616),
			0.560, 0.00000605);
		geometry.aperture(0.280);
		break;
	case CAMERA_SBIG_2800MM:
		// SBIG 16803, Cassegrain focus
		geometry = ImageGeometry(ImageSize(4096, 4096),
			2.800, 0.000015);
		geometry.aperture(0.280);
		break;
	}

	// star catalog
	CatalogPtr	catalog = CatalogFactory::get(CatalogFactory::Combined,
				std::string("/usr/local/starcatalogs"));

	// point spread function, 1 arc second of seeing
#define	ARCSECOND_IN_RADIANS	(M_PI / (180 * 60 * 60))
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sigma = %f", ARCSECOND_IN_RADIANS);
	TurbulencePointSpreadFunction	psf(2 * ARCSECOND_IN_RADIANS);
	//DiracPointSpreadFunction	psf;

	// build the factory
	ChartFactory	factory(catalog, psf, limit_mag, scale);

	// create the image
	Chart	chart = factory.chart(center, geometry);
	astro::image::ImagePtr	image = chart.image();
	astro::io::FITSout	out("tmp/chart.fits");
	out.setPrecious(false);
	out.write(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImage() end");
}

} // namespace test
} // namespace astro
