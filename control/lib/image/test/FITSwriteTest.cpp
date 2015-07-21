/*
 * FITSwriteTest.cpp -- write FITS file tests
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */

#include <AstroIO.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <iostream>
#include <AstroDebug.h>

using namespace astro::io;
using namespace astro::image;

namespace astro {
namespace test {

class FITSwriteTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testWriteUChar();
	void	testWriteUShort();
	void	testWriteYUYV();
	void	testWriteRGB();
	void	testWriteRGBUShort();

	CPPUNIT_TEST_SUITE(FITSwriteTest);
	CPPUNIT_TEST(testWriteUChar);
	CPPUNIT_TEST(testWriteUShort);
	CPPUNIT_TEST(testWriteYUYV);
	CPPUNIT_TEST(testWriteRGB);
	CPPUNIT_TEST(testWriteRGBUShort);
	CPPUNIT_TEST_SUITE_END();
};

static int	remove(const std::string& filename) {
	return ::remove(filename.c_str());
}

static const char	*uchar_filename = "uchar_test.fits";

void	FITSwriteTest::testWriteUChar() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteUChar() begin");
	// find out whether the file already exists, and destroy it
	remove(uchar_filename);

	// create an image
	Image<unsigned char>	*image = new Image<unsigned char>(256, 256);
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			image->pixel(x, y) = (x + y) % 256;
		}
	}
	FITSoutfile<unsigned char>	*outfile
		= new FITSoutfile<unsigned char>(std::string("tmp/") + uchar_filename);
	outfile->setPrecious(false);
	outfile->write(*image);
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteUChar() end");
}

static const char	*ushort_filename = "ushort_test.fits";

void	FITSwriteTest::testWriteUShort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteUShort() begin");
	// find out whether the file already exists, and destroy it
	remove(ushort_filename);

	// create an image
	Image<unsigned short>	*image = new Image<unsigned short>(256, 256);
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			image->pixel(x, y) = (x * y) % 65536;
		}
	}
	FITSoutfile<unsigned short>	*outfile
		= new FITSoutfile<unsigned short>(std::string("tmp/") +ushort_filename);
	outfile->setPrecious(false);
	outfile->write(*image);
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteUShort() end");
}

static const char	*yuyv_filename = "yuyv_test.fits";

void	FITSwriteTest::testWriteYUYV() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteYUYV() begin");
	// find out whether the file already exists, and destroy it
	remove(std::string("tmp/") + yuyv_filename);

	// create an image
	Image<YUYV<unsigned char> >	*image
		= new Image<YUYV<unsigned char> >(256, 256);
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			RGB<unsigned char>	colors[2];
			colors[0].R = x;
			colors[0].G = ((x + y) % 256);
			colors[0].B = y;
			colors[1].R = colors[0].R;
			colors[1].G = colors[0].G;
			colors[1].B = colors[0].B;
			YUYV<unsigned char>	target[2];
			convertPixelPair(target, colors);
			image->pixel(x, y).y = target[0].y;
			if (x % 2) {
				image->pixel(x, y).uv = target[0].uv;
			} else {
				image->pixel(x, y).uv = target[1].uv;
			}
		}
	}
	FITSoutfile<YUYV<unsigned char> >	*outfile
		= new FITSoutfile<YUYV<unsigned char> >(std::string("tmp/") + yuyv_filename);
	outfile->setPrecious(false);
	outfile->write(*image);
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteYUYV() end");
}

static const char	*rgb_filename = "rgb_test.fits";

void	FITSwriteTest::testWriteRGB() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteRGB() begin");
	// find out whether the file already exists, and destroy it
	remove(std::string("tmp/") + rgb_filename);

	// create an image
	Image<RGB<unsigned char> >	*image
		= new Image<RGB<unsigned char> >(256, 256);
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			image->pixel(x, y).R = x;
			image->pixel(x, y).G = (x + y) % 256;
			image->pixel(x, y).B = y;
		}
	}
	FITSoutfile<RGB<unsigned char> >	*outfile
		= new FITSoutfile<RGB<unsigned char> >(std::string("tmp/") + rgb_filename);
	outfile->setPrecious(false);
	outfile->write(*image);
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteRGB() end");
}

static const char	*rgbushort_filename = "rgbushort_test.fits";

void	FITSwriteTest::testWriteRGBUShort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteRGBUShort() begin");
	// find out whether the file already exists, and destroy it
	remove(std::string("tmp/") + rgbushort_filename);

	// create an image
	Image<RGB<unsigned short> >	*image
		= new Image<RGB<unsigned short> >(256, 256);
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			image->pixel(x, y).R = x << 8;
			image->pixel(x, y).G = ((x + y) % 256) << 8;
			image->pixel(x, y).B = y << 8;
		}
	}
	FITSoutfile<RGB<unsigned short> >	*outfile
		= new FITSoutfile<RGB<unsigned short> >(std::string("tmp/") + rgbushort_filename);
	outfile->setPrecious(false);
	outfile->write(*image);
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWriteRGBUShort() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(FITSwriteTest);

} // namespace io
} // namespace astro
