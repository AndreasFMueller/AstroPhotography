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
	for (unsigned int x = 0; x < image->size.width; x++) {
		for (unsigned int y = 0; y < image->size.height; y++) {
			CPPUNIT_ASSERT(!image->isR(x, y));
			CPPUNIT_ASSERT(!image->isG(x, y));
			CPPUNIT_ASSERT(!image->isB(x, y));
			CPPUNIT_ASSERT(!image->isGr(x, y));
			CPPUNIT_ASSERT(!image->isGb(x, y));
		}
	}
}

void	MosaicTest::testRGGB() {
	image->mosaic = ImageBase::BAYER_RGGB;
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
}

void	MosaicTest::testGRBG() {
	image->mosaic = ImageBase::BAYER_GRBG;
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
}

void	MosaicTest::testGBRG() {
	image->mosaic = ImageBase::BAYER_GBRG;
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
}

void	MosaicTest::testBGGR() {
	image->mosaic = ImageBase::BAYER_BGGR;
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
}

void	MosaicTest::testMosaic(ImageBase::mosaic_type mosaic) {
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
}

void	MosaicTest::testMosaicRGGB() {
	testMosaic(ImageBase::BAYER_RGGB);
}

void	MosaicTest::testMosaicGRBG() {
	testMosaic(ImageBase::BAYER_GRBG);
}

void	MosaicTest::testMosaicGBRG() {
	testMosaic(ImageBase::BAYER_GBRG);
}

void	MosaicTest::testMosaicBGGR() {
	testMosaic(ImageBase::BAYER_BGGR);
}

void	MosaicTest::testSeparate(ImageBase::mosaic_type mosaic) {
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
}

void	MosaicTest::testSeparateRGGB() {
	testSeparate(ImageBase::BAYER_RGGB);
}

void	MosaicTest::testSeparateGRBG() {
	testSeparate(ImageBase::BAYER_GRBG);
}

void	MosaicTest::testSeparateGBRG() {
	testSeparate(ImageBase::BAYER_GBRG);
}

void	MosaicTest::testSeparateBGGR() {
	testSeparate(ImageBase::BAYER_BGGR);
}

} // namespace test
} // namespace astro
