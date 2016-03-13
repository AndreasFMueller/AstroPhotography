/*
 * EuclideanDisplacementTest.cpp -- Test group operations
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroIO.h>
#include <AstroGroup.h>
#include <AstroImage.h>
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

class EuclideanDisplacementTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testDisplace();
	void	testInterpolate();
	void	testConvolve();

	CPPUNIT_TEST_SUITE(EuclideanDisplacementTest);
	CPPUNIT_TEST(testDisplace);
	CPPUNIT_TEST(testInterpolate);
	CPPUNIT_TEST(testConvolve);
	CPPUNIT_TEST_SUITE_END();
};

void	EuclideanDisplacementTest::testDisplace() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisplace() begin");

	// read testimages/sun.fits
	std::string	name("testimages/sun.fits");
	FITSinfile<RGB<unsigned char> >	infile(name);
        Image<RGB<unsigned char> >    *i = infile.read();
	ImagePtr	image(i);

	// apply the transformation
	Point	translation(1024 * (1 - sqrt(2)) / 2, 1024 / 2);
	transform::EuclideanDisplacement	d(M_PI / 4, translation);

	// create an image from the transform
	transform::EuclideanDisplacementAdapter<RGB<unsigned char> >	ta(*i, d);
	Image<RGB<unsigned char> >	*t = new Image<RGB<unsigned char> >(ta);
	ImagePtr	transformed(t);

	// write the transformed image back to tmp/sun-displace.fits
	std::string	outname("tmp/sun-displace.fits");
	FITSoutfile<RGB<unsigned char> >       *outfile
                = new FITSoutfile<RGB<unsigned char> >(outname);
        outfile->setPrecious(false);
        outfile->write(*t);
        delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisplace() end");
}

void	EuclideanDisplacementTest::testInterpolate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInterpolate() begin");

	// read testimages/sun.fits
	std::string	name("testimages/sun.fits");
	FITSinfile<RGB<unsigned char> >	infile(name);
        Image<RGB<unsigned char> >    *i = infile.read();
	ImagePtr	image(i);

	// apply the transformation
	Point	translation(1024 * (1 - sqrt(2)) / 2, 1024 / 2);
	transform::EuclideanDisplacement	d(M_PI / 4, translation);

	// create an image from the transform
	transform::InterpolatingEuclideanDisplacementAdapter<RGB<unsigned char> >	ta(*i, d);
	Image<RGB<unsigned char> >	*t = new Image<RGB<unsigned char> >(ta);
	ImagePtr	transformed(t);

	// write the transformed image back to tmp/sun-displace.fits
	std::string	outname("tmp/sun-interpolate.fits");
	FITSoutfile<RGB<unsigned char> >       *outfile
                = new FITSoutfile<RGB<unsigned char> >(outname);
        outfile->setPrecious(false);
        outfile->write(*t);
        delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInterpolate() end");
}

class simpleconvolution : public transform::EuclideanDisplacementFunction {
	ImageSize size;
public:
	simpleconvolution() : size(20, 20) {
	}
	virtual double	operator()(const transform::EuclideanDisplacement& d)
		const {
		if ((d.angle() < -0.1) || (d.angle() > 0.1)) {
			return 0;
		}
		if (!size.contains(floor(d.translation().x()),
			floor(d.translation().y()))) {
			return 0;
		}
		return 1;
	}
};

void	EuclideanDisplacementTest::testConvolve() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConvolve() begin");

	// read testimages/sun.fits
	std::string	name("testimages/orion1.fits");
	FITSinfile<unsigned short>	infile(name);
        Image<unsigned short>	*i = infile.read();
	ImagePtr	image(i);

	// create an adapter so that we see double values
	adapter::TypeConversionAdapter<unsigned short>	tconv(*i);

	// apply the convolution
	simpleconvolution	f;

	// create an image from the transform
	transform::EuclideanDisplacementConvolve<double>	ta(f, 1000);
	Image<double>	*result = ta(tconv);
	ImagePtr	r(result);

	// write the transformed image back to tmp/sun-displace.fits
	std::string	outname("tmp/orion-convolve.fits");
	FITSoutfile<double>       *outfile = new FITSoutfile<double>(outname);
        outfile->setPrecious(false);
        outfile->write(*result);
        delete outfile;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConvolve() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(EuclideanDisplacementTest);

} // namespace io
} // namespace astro
