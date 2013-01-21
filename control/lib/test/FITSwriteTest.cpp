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

using namespace astro::io;
using namespace astro::image;

namespace astro {
namespace test {

class FITSwriteTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testWriteUChar();
	void	testWriteChar();
	void	testWriteUShort();
	void	testWriteShort();

	CPPUNIT_TEST_SUITE(FITSwriteTest);
	CPPUNIT_TEST(testWriteUChar);
	CPPUNIT_TEST(testWriteChar);
	CPPUNIT_TEST(testWriteUShort);
	CPPUNIT_TEST(testWriteShort);
	CPPUNIT_TEST_SUITE_END();
};

const char	*uchar_filename = "uchar_test.fits";

void	FITSwriteTest::testWriteUChar() {
	// find out whether the file already exists, and destroy it
	remove(uchar_filename);

	// create an image
	Image<unsigned char>	*image = new Image<unsigned char>(256, 256);
	for (int x = 0; x < image->size.width; x++) {
		for (int y = 0; y < image->size.height; y++) {
			image->pixel(x, y) = (x + y) % 256;
		}
	}
	FITSoutfile<unsigned char>	*outfile
		= new FITSoutfile<unsigned char>(uchar_filename);
	outfile->write(*image);
	delete outfile;
}

const char	*char_filename = "char_test.fits";

void	FITSwriteTest::testWriteChar() {
	// find out whether the file already exists, and destroy it
	remove(char_filename);

	// create an image
	Image<char>	*image = new Image<char>(256, 256);
	for (int x = 0; x < image->size.width; x++) {
		for (int y = 0; y < image->size.height; y++) {
			image->pixel(x, y) = (x - y) % 256;
		}
	}
	FITSoutfile<char>	*outfile
		= new FITSoutfile<char>(char_filename);
	outfile->write(*image);
	delete outfile;
}

const char	*ushort_filename = "ushort_test.fits";

void	FITSwriteTest::testWriteUShort() {
	// find out whether the file already exists, and destroy it
	remove(ushort_filename);

	// create an image
	Image<unsigned short>	*image = new Image<unsigned short>(256, 256);
	for (int x = 0; x < image->size.width; x++) {
		for (int y = 0; y < image->size.height; y++) {
			image->pixel(x, y) = (x * y) % 65536;
		}
	}
	FITSoutfile<unsigned short>	*outfile
		= new FITSoutfile<unsigned short>(ushort_filename);
	outfile->write(*image);
	delete outfile;
}

const char	*short_filename = "short_test.fits";

void	FITSwriteTest::testWriteShort() {
	// find out whether the file already exists, and destroy it
	remove(short_filename);

	// create an image
	Image<short>	*image = new Image<short>(256, 256);
	for (int x = 0; x < image->size.width; x++) {
		for (int y = 0; y < image->size.height; y++) {
			image->pixel(x, y) = (x * y) % 65536;
		}
	}
	FITSoutfile<short>	*outfile
		= new FITSoutfile<short>(short_filename);
	outfile->write(*image);
	delete outfile;
}

CPPUNIT_TEST_SUITE_REGISTRATION(FITSwriteTest);

} // namespace io
} // namespace astro
