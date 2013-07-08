/*
 * MosaicTests.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <AstroMosaic.h>
#include <AstroDemosaic.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroIO.h>
#include <Format.h>
#include <includes.h>
#include <debug.h>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace test {

class MosaicTest : public CppUnit::TestFixture {
private:
	Image<unsigned char>	*image;
public:
	void	setUp();
	void	tearDown();
	void	testNone();
	void	testRGGB();
	void	testGRBG();
	void	testGBRG();
	void	testBGGR();
	void	testMosaic(ImageBase::mosaic_type mosaic);
	void	testMosaicRGGB();
	void	testMosaicGRBG();
	void	testMosaicGBRG();
	void	testMosaicBGGR();
	void	testSeparate(ImageBase::mosaic_type mosaic);
	void	testSeparateRGGB();
	void	testSeparateGRBG();
	void	testSeparateGBRG();
	void	testSeparateBGGR();
	void	testDemosaicBilinear();

	CPPUNIT_TEST_SUITE(MosaicTest);
	CPPUNIT_TEST(testNone);
	CPPUNIT_TEST(testRGGB);
	CPPUNIT_TEST(testGRBG);
	CPPUNIT_TEST(testGBRG);
	CPPUNIT_TEST(testBGGR);
	CPPUNIT_TEST(testMosaicRGGB);
	CPPUNIT_TEST(testMosaicGRBG);
	CPPUNIT_TEST(testMosaicGBRG);
	CPPUNIT_TEST(testMosaicBGGR);
	CPPUNIT_TEST(testSeparateRGGB);
	CPPUNIT_TEST(testSeparateGRBG);
	CPPUNIT_TEST(testSeparateGBRG);
	CPPUNIT_TEST(testSeparateBGGR);
	CPPUNIT_TEST(testDemosaicBilinear);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MosaicTest);

void	MosaicTest::setUp() {
	image = new Image<unsigned char>(64, 48);
}

void	MosaicTest::tearDown() {
	delete image;
}

void	MosaicTest::testNone() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNone() begin");
	for (unsigned int x = 0; x < image->size.width; x++) {
		for (unsigned int y = 0; y < image->size.height; y++) {
			CPPUNIT_ASSERT(!image->isR(x, y));
			CPPUNIT_ASSERT(!image->isG(x, y));
			CPPUNIT_ASSERT(!image->isB(x, y));
			CPPUNIT_ASSERT(!image->isGr(x, y));
			CPPUNIT_ASSERT(!image->isGb(x, y));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNone() end");
}

void	MosaicTest::testRGGB() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGGB() begin");
	image->setMosaicType(ImageBase::BAYER_RGGB);
	for (unsigned int x = 0; x < image->size.width; x += 2) {
		for (unsigned int y = 0; y < image->size.height; y += 2) {
			// point (0,0)
			CPPUNIT_ASSERT( image->isR(x, y));
			CPPUNIT_ASSERT(!image->isG(x, y));
			CPPUNIT_ASSERT(!image->isB(x, y));
			CPPUNIT_ASSERT(!image->isGr(x, y));
			CPPUNIT_ASSERT(!image->isGb(x, y));
			// point (1,0)
			CPPUNIT_ASSERT(!image->isR(x + 1, y));
			CPPUNIT_ASSERT( image->isG(x + 1, y));
			CPPUNIT_ASSERT(!image->isB(x + 1, y));
			CPPUNIT_ASSERT( image->isGr(x + 1, y));
			CPPUNIT_ASSERT(!image->isGb(x + 1, y));
			// point (0,1)
			CPPUNIT_ASSERT(!image->isR(x, y + 1));
			CPPUNIT_ASSERT( image->isG(x, y + 1));
			CPPUNIT_ASSERT(!image->isB(x, y + 1));
			CPPUNIT_ASSERT(!image->isGr(x, y + 1));
			CPPUNIT_ASSERT( image->isGb(x, y + 1));
			// point (1,1)
			CPPUNIT_ASSERT(!image->isR(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isG(x + 1, y + 1));
			CPPUNIT_ASSERT( image->isB(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isGr(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isGb(x + 1, y + 1));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRGGB() end");
}

void	MosaicTest::testGRBG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGRBG() begin");
	image->setMosaicType(ImageBase::BAYER_GRBG);
	for (unsigned int x = 0; x < image->size.width; x += 2) {
		for (unsigned int y = 0; y < image->size.height; y += 2) {
			// point (0,0): G, Gr
			CPPUNIT_ASSERT(!image->isR(x, y));
			CPPUNIT_ASSERT( image->isG(x, y));
			CPPUNIT_ASSERT(!image->isB(x, y));
			CPPUNIT_ASSERT( image->isGr(x, y));
			CPPUNIT_ASSERT(!image->isGb(x, y));
			// point (1,0): R
			CPPUNIT_ASSERT( image->isR(x + 1, y));
			CPPUNIT_ASSERT(!image->isG(x + 1, y));
			CPPUNIT_ASSERT(!image->isB(x + 1, y));
			CPPUNIT_ASSERT(!image->isGr(x + 1, y));
			CPPUNIT_ASSERT(!image->isGb(x + 1, y));
			// point (0,1): B
			CPPUNIT_ASSERT(!image->isR(x, y + 1));
			CPPUNIT_ASSERT(!image->isG(x, y + 1));
			CPPUNIT_ASSERT( image->isB(x, y + 1));
			CPPUNIT_ASSERT(!image->isGr(x, y + 1));
			CPPUNIT_ASSERT(!image->isGb(x, y + 1));
			// point (1,1): G, Gb
			CPPUNIT_ASSERT(!image->isR(x + 1, y + 1));
			CPPUNIT_ASSERT( image->isG(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isB(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isGr(x + 1, y + 1));
			CPPUNIT_ASSERT( image->isGb(x + 1, y + 1));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGRBG() end");
}

void	MosaicTest::testGBRG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGBRG() begin");
	image->setMosaicType(ImageBase::BAYER_GBRG);
	for (unsigned int x = 0; x < image->size.width; x += 2) {
		for (unsigned int y = 0; y < image->size.height; y += 2) {
			// point (0,0): G, Gb
			CPPUNIT_ASSERT(!image->isR(x, y));
			CPPUNIT_ASSERT( image->isG(x, y));
			CPPUNIT_ASSERT(!image->isB(x, y));
			CPPUNIT_ASSERT(!image->isGr(x, y));
			CPPUNIT_ASSERT( image->isGb(x, y));
			// point (1,0): B
			CPPUNIT_ASSERT(!image->isR(x + 1, y));
			CPPUNIT_ASSERT(!image->isG(x + 1, y));
			CPPUNIT_ASSERT( image->isB(x + 1, y));
			CPPUNIT_ASSERT(!image->isGr(x + 1, y));
			CPPUNIT_ASSERT(!image->isGb(x + 1, y));
			// point (0,1): R
			CPPUNIT_ASSERT( image->isR(x, y + 1));
			CPPUNIT_ASSERT(!image->isG(x, y + 1));
			CPPUNIT_ASSERT(!image->isB(x, y + 1));
			CPPUNIT_ASSERT(!image->isGr(x, y + 1));
			CPPUNIT_ASSERT(!image->isGb(x, y + 1));
			// point (1,1): G, Gr
			CPPUNIT_ASSERT(!image->isR(x + 1, y + 1));
			CPPUNIT_ASSERT( image->isG(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isB(x + 1, y + 1));
			CPPUNIT_ASSERT( image->isGr(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isGb(x + 1, y + 1));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGBRG() end");
}

void	MosaicTest::testBGGR() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBGGR() begin");
	image->setMosaicType(ImageBase::BAYER_BGGR);
	for (unsigned int x = 0; x < image->size.width; x += 2) {
		for (unsigned int y = 0; y < image->size.height; y += 2) {
			// point (0,0): B
			CPPUNIT_ASSERT(!image->isR(x, y));
			CPPUNIT_ASSERT(!image->isG(x, y));
			CPPUNIT_ASSERT( image->isB(x, y));
			CPPUNIT_ASSERT(!image->isGr(x, y));
			CPPUNIT_ASSERT(!image->isGb(x, y));
			// point (1,0): G, Gb
			CPPUNIT_ASSERT(!image->isR(x + 1, y));
			CPPUNIT_ASSERT( image->isG(x + 1, y));
			CPPUNIT_ASSERT(!image->isB(x + 1, y));
			CPPUNIT_ASSERT(!image->isGr(x + 1, y));
			CPPUNIT_ASSERT( image->isGb(x + 1, y));
			// point (0,1): G, Gr
			CPPUNIT_ASSERT(!image->isR(x, y + 1));
			CPPUNIT_ASSERT( image->isG(x, y + 1));
			CPPUNIT_ASSERT(!image->isB(x, y + 1));
			CPPUNIT_ASSERT( image->isGr(x, y + 1));
			CPPUNIT_ASSERT(!image->isGb(x, y + 1));
			// point (1,1): R
			CPPUNIT_ASSERT( image->isR(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isG(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isB(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isGr(x + 1, y + 1));
			CPPUNIT_ASSERT(!image->isGb(x + 1, y + 1));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBGGR() end");
}

void	MosaicTest::testMosaic(ImageBase::mosaic_type mosaic) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaic() begin");
	Image<RGB<unsigned char> >	image(44, 62);
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			image.pixel(x, y).R = 'R';
			image.pixel(x, y).G = 'G';
			image.pixel(x, y).B = 'B';
		}
	}

	Mosaic<unsigned char>	mosaicer(mosaic);
	Image<unsigned char>	*mosaiced = mosaicer(image);
	std::string	filename = stringprintf("mosaic%d.fits", mosaic);
	unlink(filename.c_str());
	FITSoutfile<unsigned char>	*outfile
		= new FITSoutfile<unsigned char>(filename.c_str());
	outfile->write(*mosaiced);
        delete outfile;

	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			if (mosaiced->isR(x, y)) {
				CPPUNIT_ASSERT(mosaiced->pixel(x, y) == 'R');
			}
			if (mosaiced->isG(x, y)) {
				CPPUNIT_ASSERT(mosaiced->pixel(x, y) == 'G');
			}
			if (mosaiced->isB(x, y)) {
				CPPUNIT_ASSERT(mosaiced->pixel(x, y) == 'B');
			}
		}
	}
	delete mosaiced;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaic() end");
}

void	MosaicTest::testMosaicRGGB() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaicRGGB() begin");
	testMosaic(ImageBase::BAYER_RGGB);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaicRGGB() end");
}

void	MosaicTest::testMosaicGRBG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaicGRBG() begin");
	testMosaic(ImageBase::BAYER_GRBG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaicGRBG() end");
}

void	MosaicTest::testMosaicGBRG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaicGBRG() begin");
	testMosaic(ImageBase::BAYER_GBRG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaicGBRG() end");
}

void	MosaicTest::testMosaicBGGR() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaicBGGR() begin");
	testMosaic(ImageBase::BAYER_BGGR);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMosaicBGGR() end");
}

void	MosaicTest::testSeparate(ImageBase::mosaic_type mosaic) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparate() begin");
	Image<RGB<unsigned char> >	image(44, 62);
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			image.pixel(x, y).R = 'R';
			image.pixel(x, y).G = 'G';
			image.pixel(x, y).B = 'B';
		}
	}

	Mosaic<unsigned char>	mosaicer(mosaic);
	Image<unsigned char>	*mosaiced = mosaicer(image);

	Demosaic<unsigned char>	demosaic;

	Image<RGB<unsigned char> >	*demosaiced = demosaic(*mosaiced);
	CPPUNIT_ASSERT((demosaiced->size.width == 44)
		&& (demosaiced->size.height == 62));
	delete mosaiced;
	
	for (unsigned int x = 0; x < image.size.width; x++) {
		for (unsigned int y = 0; y < image.size.height; y++) {
			CPPUNIT_ASSERT((demosaiced->pixel(x, y).R == 0)
				|| (demosaiced->pixel(x, y).R == 'R'));
			CPPUNIT_ASSERT((demosaiced->pixel(x, y).G == 0)
				|| (demosaiced->pixel(x, y).G == 'G'));
			CPPUNIT_ASSERT((demosaiced->pixel(x, y).B == 0)
				|| (demosaiced->pixel(x, y).B == 'B'));
		}
	}

	delete demosaiced;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparate() end");
}

void	MosaicTest::testSeparateRGGB() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparateRGGB() begin");
	testSeparate(ImageBase::BAYER_RGGB);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparateRGGB() end");
}

void	MosaicTest::testSeparateGRBG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparateGRBG() begin");
	testSeparate(ImageBase::BAYER_GRBG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparateGRBG() end");
}

void	MosaicTest::testSeparateGBRG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparateGBRG() begin");
	testSeparate(ImageBase::BAYER_GBRG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparateGBRG() end");
}

void	MosaicTest::testSeparateBGGR() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparateBGGR() begin");
	testSeparate(ImageBase::BAYER_BGGR);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSeparateBGGR() end");
}

void	MosaicTest::testDemosaicBilinear() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDemosaicBilinear() begin");
	Image<RGB<unsigned char> >	*image
		= new Image<RGB<unsigned char> >(80, 80);
	ImagePtr	imageptr(image);
	for (unsigned int x = 0; x < image->size.width; x++) {
		for (unsigned int y = 0; y < image->size.height; y++) {
			int	v = ((x / 10) + (y / 10)) % 8;
			image->pixel(x, y).R = (v & 0x4) ? 0xff : 0;
			image->pixel(x, y).G = (v & 0x2) ? 0xff : 0;
			image->pixel(x, y).B = (v & 0x1) ? 0xff : 0;
		}
	}

	/* separate into bayer pattern */
	Mosaic<unsigned char>	mosaicer(ImageBase::BAYER_GRBG);
	Image<unsigned char>	*mosaiced = mosaicer(*image);
	ImagePtr	mosaicimage(mosaiced);
	unlink("rgbmosaic.fits");
	FITSout	mosaicfile("rgbmosaic.fits");
	mosaicfile.write(mosaicimage);

	/* demosaic */
	DemosaicBilinear<unsigned char>	demosaicer;
	Image<RGB<unsigned char> >	*rgb = demosaicer(*mosaiced);
	ImagePtr	rgbptr(rgb);
	unlink("rgb.fits");
	FITSout	file("rgb.fits");
	file.write(rgbptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDemosaicBilinear() end");
}

} // namespace test
} // namespace astro
