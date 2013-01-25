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

	CPPUNIT_TEST_SUITE(FITSreadTest);
	CPPUNIT_TEST(testReadUChar);
	CPPUNIT_TEST(testReadUShort);
	CPPUNIT_TEST(testReadYUYV);
	CPPUNIT_TEST(testReadRGB);
	CPPUNIT_TEST(testReadRGBUShort);
	CPPUNIT_TEST_SUITE_END();
};

const char	*uchar_filename = "uchar_test.fits";

void	FITSreadTest::testReadUChar() {
	FITSinfile<unsigned char>	infile(uchar_filename);
	std::tr1::shared_ptr<Image<unsigned char> >	i = infile.read();
	std::cerr << "image type: " << infile.getImgtype() << std::endl;
	std::cerr << "size: " << i->size.width << " x " << i->size.height << std::endl;
	std::cerr << "planes: " << infile.getPlanes() << std::endl;
	std::tr1::shared_ptr<Image<unsigned char> >	image = infile.read();
	std::string	newfilename = std::string("copy-") + uchar_filename;
	remove(newfilename.c_str());
	FITSoutfile<unsigned char>	*outfile
		= new FITSoutfile<unsigned char>(newfilename);
	outfile->write(*image);
	delete outfile;
}

const char	*ushort_filename = "ushort_test.fits";

void	FITSreadTest::testReadUShort() {
	FITSinfile<unsigned short>	infile(ushort_filename);
	std::cerr << "image type: " << infile.getImgtype() << std::endl;
	std::cerr << "size: " << infile.getSize().width << " x "
		<< infile.getSize().height << std::endl;
	std::cerr << "planes: " << infile.getPlanes() << std::endl;
	std::cerr << "read pixels" << std::endl;
	std::tr1::shared_ptr<Image<unsigned short> >	image = infile.read();
	std::cerr << "image read" << std::endl;

	std::string	newfilename = std::string("copy-") + ushort_filename;
	remove(newfilename.c_str());
	FITSoutfile<unsigned short>	*outfile
		= new FITSoutfile<unsigned short>(newfilename);
	outfile->write(*image);
	delete outfile;
}

const char	*yuyv_filename = "yuyv_test.fits";

void	FITSreadTest::testReadYUYV() {
	FITSinfile<YUYV<unsigned char> >	infile(yuyv_filename);
	std::cerr << "image type: " << infile.getImgtype() << std::endl;
	std::cerr << "size: " << infile.getSize().width << " x "
		<< infile.getSize().height << std::endl;
	std::cerr << "planes: " << infile.getPlanes() << std::endl;
	std::tr1::shared_ptr<Image<YUYV<unsigned char> > >	image
		= infile.read();

	std::string	newfilename = std::string("copy-") + yuyv_filename;
	remove(newfilename.c_str());
	FITSoutfile<YUYV<unsigned char> >	*outfile
		= new FITSoutfile<YUYV<unsigned char> >(newfilename);
	outfile->write(*image);
	delete outfile;
}

const char	*rgb_filename = "rgb_test.fits";

void	FITSreadTest::testReadRGB() {
	FITSinfile<RGB<unsigned char> >	infile(rgb_filename);
	std::cerr << "image type: " << infile.getImgtype() << std::endl;
	std::cerr << "size: " << infile.getSize().width << " x " << infile.getSize().height << std::endl;
	std::cerr << "planes: " << infile.getPlanes() << std::endl;
	std::tr1::shared_ptr<Image<RGB<unsigned char> > >	image
		= infile.read();
	std::string	newfilename = std::string("copy-") + rgb_filename;
	remove(newfilename.c_str());
	FITSoutfile<RGB<unsigned char> >	*outfile
		= new FITSoutfile<RGB<unsigned char> >(newfilename);
	outfile->write(*image);
	delete outfile;
}

const char	*rgbushort_filename = "rgbushort_test.fits";

void	FITSreadTest::testReadRGBUShort() {
	FITSinfile<RGB<unsigned short> >	infile(rgbushort_filename);
	std::cerr << "image type: " << infile.getImgtype() << std::endl;
	std::cerr << "size: " << infile.getSize().width << " x " << infile.getSize().height << std::endl;
	std::cerr << "planes: " << infile.getPlanes() << std::endl;
	std::tr1::shared_ptr<Image<RGB<unsigned short> > >	image
		= infile.read();
	std::string	newfilename = std::string("copy-") + rgbushort_filename;
	remove(newfilename.c_str());
	FITSoutfile<RGB<unsigned short> >	*outfile
		= new FITSoutfile<RGB<unsigned short> >(newfilename);
	outfile->write(*image);
	delete outfile;
}

CPPUNIT_TEST_SUITE_REGISTRATION(FITSreadTest);

} // namespace io
} // namespace astro
