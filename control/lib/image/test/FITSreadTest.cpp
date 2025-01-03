/*
 * FITSreadTest.cpp -- read FITS file tests
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
#include <AstroAdapter.h>

using namespace astro::io;
using namespace astro::image;

namespace astro {
namespace test {

class FITSreadTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testReadUChar();
	void	testReadUShort();
	void	testReadYUYV();
	void	testReadRGB();
	void	testReadRGBUShort();
	void	testReadXYZ();

	CPPUNIT_TEST_SUITE(FITSreadTest);
	CPPUNIT_TEST(testReadUChar);
	CPPUNIT_TEST(testReadUShort);
	CPPUNIT_TEST(testReadYUYV);
	CPPUNIT_TEST(testReadRGB);
	CPPUNIT_TEST(testReadRGBUShort);
	CPPUNIT_TEST(testReadXYZ);
	CPPUNIT_TEST_SUITE_END();
};

const char	*uchar_filename = "uchar_test.fits";

void	FITSreadTest::testReadUChar() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadUChar() begin");
	FITSinfile<unsigned char>	infile(std::string("tmp/") + uchar_filename);
	Image<unsigned char>	*i = infile.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image type: %d", infile.getImgtype());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size. %d x %d",
		i->size().width(), i->size().height());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "planes: %d", infile.getPlanes());
	delete	i;
	Image<unsigned char>	*image = infile.read();
	std::string	newfilename = std::string("tmp/copy-") + uchar_filename;
	remove(newfilename.c_str());
	FITSoutfile<unsigned char>	*outfile
		= new FITSoutfile<unsigned char>(newfilename);
	outfile->write(*image);
	delete image;
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadUChar() end");
}

const char	*ushort_filename = "ushort_test.fits";

void	FITSreadTest::testReadUShort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadUShort() begin");
	FITSinfile<unsigned short>	infile(std::string("tmp/") + ushort_filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image type: %d", infile.getImgtype());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %d x %d", infile.getSize().width(),
		infile.getSize().height());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "planes: %d", infile.getPlanes());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "read pixels");
	Image<unsigned short>	*image = infile.read();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image read");

	std::string	newfilename = std::string("tmp/copy-") + ushort_filename;
	remove(newfilename.c_str());
	FITSoutfile<unsigned short>	*outfile
		= new FITSoutfile<unsigned short>(newfilename);
	outfile->write(*image);
	delete image;
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadUShort() end");
}

const char	*yuyv_filename = "yuyv_test.fits";

void	FITSreadTest::testReadYUYV() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadYUYV() begin");
	FITSinfile<YUYV<unsigned char> >	infile(std::string("tmp/") + yuyv_filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image type: %d", infile.getImgtype());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %d x %d", infile.getSize().width(),
		infile.getSize().height());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "planes: %d", infile.getPlanes());
	Image<YUYV<unsigned char> >	*image = infile.read();

	std::string	newfilename = std::string("tmp/copy-") + yuyv_filename;
	remove(newfilename.c_str());
	FITSoutfile<YUYV<unsigned char> >	*outfile
		= new FITSoutfile<YUYV<unsigned char> >(newfilename);
	outfile->write(*image);
	delete image;
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadYUYV() end");
}

const char	*rgb_filename = "rgb_test.fits";

void	FITSreadTest::testReadRGB() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadRGB() begin");
	FITSinfile<RGB<unsigned char> >	infile(std::string("tmp/") + rgb_filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image type: %d", infile.getImgtype());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %d x %d", infile.getSize().width(),
		infile.getSize().height());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "planes: %d", infile.getPlanes());
	Image<RGB<unsigned char> >	*image = infile.read();
	std::string	newfilename = std::string("tmp/copy-") + rgb_filename;
	remove(newfilename.c_str());
	FITSoutfile<RGB<unsigned char> >	*outfile
		= new FITSoutfile<RGB<unsigned char> >(newfilename);
	outfile->write(*image);
	delete image;
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadRGB() end");
}

const char	*rgbushort_filename = "rgbushort_test.fits";

void	FITSreadTest::testReadRGBUShort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadRGBUShort() begin");
	FITSinfile<RGB<unsigned short> >	infile(std::string("tmp/") + rgbushort_filename);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image type: ", infile.getImgtype());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %d x %d", infile.getSize().width(),
		infile.getSize().height());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "planes: %d", infile.getPlanes());
	Image<RGB<unsigned short> >	*image = infile.read();
	std::string	newfilename = std::string("tmp/copy-") + rgbushort_filename;
	remove(newfilename.c_str());
	FITSoutfile<RGB<unsigned short> >	*outfile
		= new FITSoutfile<RGB<unsigned short> >(newfilename);
	outfile->write(*image);
	delete image;
	delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadRGBUShort() end");
}

void	FITSreadTest::testReadXYZ() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadXYZ() begin");
	FITSin	in("3.fits");
	ImagePtr	img = in.read();
	Image<XYZ<unsigned short> >	*xyzimg
		= dynamic_cast<Image<XYZ<unsigned short> >*>(&*img);
	if (NULL != xyzimg) {
		adapter::ColorConversionAdapter<RGB<float>, XYZ<unsigned short> > cc(*xyzimg);
		Image<RGB<float> >	*rgbimg = new Image<RGB<float> >(cc);
		FITSoutfile<RGB<float> >	outfile("3-rgb.fits");
		outfile.setPrecious(false);
		outfile.write(*rgbimg);
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "not an XYZ image");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testReadXYZ() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(FITSreadTest);

} // namespace io
} // namespace astro
