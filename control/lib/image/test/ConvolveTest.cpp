/*
 * ConvolveTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroConvolve.h>
#include <AstroAdapter.h>
#include <AstroIO.h>

using namespace astro::image;

namespace astro {
namespace test {

class ConvolveTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testConvolution();
	void	testInteger();
	void	testAbs();
	void	testPhase();
	void	testColor();
	void	testConvolutionResult();
	void	testAiry();
	void	testGauss();
	void	testDisk();
	void	testUranus();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ConvolveTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testConvolution);
	CPPUNIT_TEST(testInteger);
	CPPUNIT_TEST(testAbs);
	CPPUNIT_TEST(testPhase);
	CPPUNIT_TEST(testColor);
	CPPUNIT_TEST(testConvolutionResult);
	CPPUNIT_TEST(testAiry);
	CPPUNIT_TEST(testGauss);
	CPPUNIT_TEST(testDisk);
	CPPUNIT_TEST(testUranus);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConvolveTest);

void	ConvolveTest::setUp() {
}

void	ConvolveTest::tearDown() {
}

void	ConvolveTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	Image<double>	in(100, 101);
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 101; y++) {
			in.pixel(x,y) = (x + y) % 256;
		}
	}
	FourierImage	fimage(in);
	ImagePtr	img = fimage.inverse();
	Image<double>	*inverse = dynamic_cast<Image<double>*>(&*img);
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 101; y++) {
#if 0
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d: %f %f %f",
				x, y, in.pixel(x,y), inverse->pixel(x,y),
				in.pixel(x,y) - inverse->pixel(x,y));
#endif
			CPPUNIT_ASSERT(fabs(in.pixel(x, y)
				- inverse->pixel(x,y)) < 1);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	ConvolveTest::testConvolution() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConvolution() begin");
	Image<double>	in(100, 101);
	Image<double>	psf(100, 101);
	for (size_t i = 0; i < in.size().getPixels(); i++) {
		in.pixels[i] = 0.;
		psf.pixels[i] = 0.;
	}
	in.pixel(0, 0) = 1;
	in.pixel(30, 80) = 1;
	for (int x = 49; x <= 51; x++) {
		for (int y = 49; y <= 51; y++) {
			psf.pixel(x, y) = 1;
		}
	}
	FourierImage	a(in);
	FourierImage	b(psf);
	FourierImagePtr	result = a * b;
	ImagePtr	img = result->inverse();
	Image<double>	*image = dynamic_cast<Image<double>*>(&*img);
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 101; y++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d %f", x, y,
				image->pixel(x,y));
			if (((49 <= x) && (x <= 51) && (49 <= y) && (y <= 51))
				|| ((79 <= x) && (x <= 81) && (28 <= y) && (y <= 30))) {

				CPPUNIT_ASSERT(fabs(image->pixel(x, y) - 1) < 0.1);
			} else {
				CPPUNIT_ASSERT(fabs(image->pixel(x, y)) < 0.1);
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConvolution() end");
}

void	ConvolveTest::testInteger() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInteger() begin");
	Image<unsigned char>	in(100, 200);
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 200; y++) {
			unsigned char	v = (x + y) % 256;
			in.pixel(x,y) = v;
		}
	}
	FourierImage	fimage(in);
	ImagePtr	img = fimage.inverse();
	Image<double>	*inverse = dynamic_cast<Image<double>*>(&*img);
	for (int x = 0; x < 100; x++) {
		for (int y = 0; y < 200; y++) {
#if 0
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d: %f %f %f",
				x, y, in.pixel(x,y), inverse->pixel(x,y),
				in.pixel(x,y) - inverse->pixel(x,y));
#endif
			CPPUNIT_ASSERT(fabs(in.pixel(x, y)
				- inverse->pixel(x,y)) < 1);
		}
	}
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInteger() end");
}

void	ConvolveTest::testAbs() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAbs() begin");
	Image<unsigned char>	in(1000, 1047);
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1047; y++) {
			unsigned char	v = (hypot(x - 500, y - 500) < 5) ? 1 : 0;
			in.pixel(x, y) = v;
		}
	}
	FourierImage	a(in);
	ImagePtr	b = a.abs();
	io::FITSout	out("tmp/convolve-test-abs.fits");
	out.setPrecious(false);
        out.write(b);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAbs() end");
}

void	ConvolveTest::testPhase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPhase() begin");
	Image<unsigned char>	in(1000, 1047);
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1047; y++) {
			unsigned char	v = (hypot(x - 500, y - 500) < 5) ? 1 : 0;
			in.pixel(x, y) = v;
		}
	}
	FourierImage	a(in);
	ImagePtr	b = a.phase();
	io::FITSout	out("tmp/convolve-test-phase.fits");
	out.setPrecious(false);
        out.write(b);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPhase() end");
}

void	ConvolveTest::testColor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testColor() begin");
	Image<unsigned char>	in(1000, 1047);
	for (int x = 0; x < 1000; x++) {
		for (int y = 0; y < 1047; y++) {
			int	dx = (x > 500) ? (1000 - x) : x;
			int	dy = (y > 500) ? (1047 - y) : y;
			unsigned char	v = (hypot(dx, dy) < 5) ? 1 : 0;
			in.pixel(x, y) = v;
		}
	}
	FourierImage	a(in);
	ImagePtr	b = a.color();
	io::FITSout	out("tmp/convolve-test-color.fits");
	out.setPrecious(false);
        out.write(b);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testColor() end");
}

void	ConvolveTest::testConvolutionResult() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConvolutionResult() begin");
	Image<unsigned char>	in(47, 53);
	Image<unsigned char>	op(47, 53);
	for (int x = 0; x < 47; x++) {
		for (int y = 0; y < 53; y++) {
			unsigned char	v = (x + y) % 256;
			in.pixel(x, y) = v;
			op.pixel(x, y) = 0;
		}
	}
	op.pixel(20, 30) = 1;
	ConvolutionResult	a(in, Point(0, 0));
	ConvolutionResult	b(op, Point(0, 0));
	ConvolutionResultPtr	c = a * b;
	ImagePtr	imageptr = c->image();
	Image<double>	*imagep = dynamic_cast<Image<double>*>(&*imageptr);
	adapter::RollAdapter<unsigned char>	roll(in, ImagePoint(20, 30));
	for (int x = 0; x < 47; x++) {
		for (int y = 0; y < 53; y++) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%d,%d %f %d %c",
				x, y, imagep->pixel(x, y), roll.pixel(x, y),
				(fabs(imagep->pixel(x, y) - roll.pixel(x, y)) > 0.01) ?  '*' : ' ');
			//CPPUNIT_ASSERT(fabs(imagep->pixel(x, y) - roll.pixel(x, y)) < 0.01);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConvolutionResult() end");
}

void	ConvolveTest::testAiry() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAiry() begin");
	AiryImage	a(ImageSize(2048,2048), ImagePoint(1024,1024), 0.044,
		0.01 * 6.5e-6 / 2.800);
	Image<double>	*b = new Image<double>(a);
	ImagePtr	c(b);
	io::FITSout	out("tmp/airy.fits");
	out.setPrecious(false);
	out.write(c);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAiry() end");
}

void	ConvolveTest::testGauss() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGauss() begin");
	GaussImage	g(ImageSize(2048,2048), ImagePoint(1024,1024),
				(M_PI / 180) * 18.4 / 3600, 
				6.5e-6 / 2.800);
	Image<double>	*b = new Image<double>(g);
	ImagePtr	c(b);
	io::FITSout	out("tmp/gauss.fits");
	out.setPrecious(false);
	out.write(c);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testGauss() end");
}

static double	scale = 0.1;

void	ConvolveTest::testDisk() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisk() begin");

	DiskImage	mars(ImageSize(2048,2048), ImagePoint(1024,1024),
				(M_PI / 180) * 25.1 / 3600,
				scale * 6.5e-6 / 2.800);
	ImagePtr	marsptr(new Image<double>(mars));

	io::FITSout	marsout("tmp/mars.fits");
	marsout.setPrecious(false);
	marsout.write(marsptr);
		
	DiskImage	uranus(ImageSize(2048,2048), ImagePoint(1024,1024),
				(M_PI / 180) * 4.1 / 3600,
				scale * 6.5e-6 / 2.800);
	ImagePtr	uranusptr(new Image<double>(uranus));

	io::FITSout	uranusout("tmp/uranus.fits");
	uranusout.setPrecious(false);
	uranusout.write(uranusptr);
		
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDisk() end");
}

void	ConvolveTest::testUranus() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testUranus() begin");
	double	resolution = 0.2 * (M_PI / 180) * 18.4 / 3600;
	double	herschel_improvement = 0.25;

	AiryImage	airy(ImageSize(2048,2048), ImagePoint(1024,1024), 0.075,
				scale * 6.5e-6 / 2.800);
	ConvolutionResult	ai(airy, Point(1024, 1024));
	GaussImage	gauss(ImageSize(2048,2048), ImagePoint(1024,1024),
				resolution,
				scale * 6.5e-6 / 2.800);
	GaussImage	herschel(ImageSize(2048,2048), ImagePoint(1024,1024),
				herschel_improvement * resolution, 
				scale * 6.5e-6 / 2.800);
	DiskImage	mars(ImageSize(2048,2048), ImagePoint(1024,1024),
				(M_PI / 180) * 25.1 / 3600,
				scale * 6.5e-6 / 2.800);
	ImagePtr	marsptr(new Image<double>(mars));

	DiskImage	uranus(ImageSize(2048,2048), ImagePoint(1024,1024),
				(M_PI / 180) * 4.1 / 3600,
				scale * 6.5e-6 / 2.800);
	ImagePtr	uranusptr(new Image<double>(uranus));

	{
		ImagePtr	ptr(new Image<double>(airy));
		io::FITSout	out("tmp/star-airy.fits");
		out.setPrecious(false);
		out.write(ptr);
	}
	{
		ConvolutionResult	b(gauss, Point(1024, 1024));
		ConvolutionResultPtr	c = ai * b;
		ImagePtr	ptr(new Image<double>(gauss));
		ImagePtr	imageptr = c->image();
		Image<double>	*imagep = dynamic_cast<Image<double>*>(&*imageptr);
		adapter::RollAdapter<double>	roll(*imagep, ImagePoint(1024, 1024));
		ImagePtr	rolled(new Image<double>(roll));
		io::FITSout	out("tmp/star-gauss.fits");
		out.setPrecious(false);
		out.write(rolled);
	}
	{
		ConvolutionResult	b(herschel, Point(1024, 1024));
		ConvolutionResultPtr	c = ai * b;
		ImagePtr	ptr(new Image<double>(gauss));
		ImagePtr	imageptr = c->image();
		Image<double>	*imagep = dynamic_cast<Image<double>*>(&*imageptr);
		adapter::RollAdapter<double>	roll(*imagep, ImagePoint(1024, 1024));
		ImagePtr	rolled(new Image<double>(roll));
		io::FITSout	out("tmp/star-herschel.fits");
		out.setPrecious(false);
		out.write(rolled);
	}
	{
		ConvolutionResult	a(mars, Point(1024, 1024));
		ConvolutionResult	b(gauss, Point(1024, 1024));
		ConvolutionResultPtr	c = ai * a * b;
		ImagePtr	imageptr = c->image();
#if 0
		Image<double>	*imagep = dynamic_cast<Image<double>*>(&*imageptr);
		adapter::RollAdapter<double>	roll(*imagep, ImagePoint(1024, 1024));
		ImagePtr	rolled = ImagePtr(new Image<double>(roll));
#endif

		io::FITSout	out("tmp/mars-gauss.fits");
		out.setPrecious(false);
		out.write(imageptr);
	}
	{
		ConvolutionResult	a(mars, Point(1024, 1024));
		ConvolutionResult	b(herschel, Point(1024, 1024));
		ConvolutionResultPtr	c = ai * a * b;
		ImagePtr	imageptr = c->image();
		Image<double>	*imagep = dynamic_cast<Image<double>*>(&*imageptr);
		adapter::RollAdapter<double>	roll(*imagep, ImagePoint(1024, 1024));
		ImagePtr	rolled = ImagePtr(new Image<double>(roll));

		io::FITSout	out("tmp/mars-herschel.fits");
		out.setPrecious(false);
		out.write(imageptr);
	}
	{
		ConvolutionResult	a(uranus, Point(1024, 1024));
		ConvolutionResult	b(gauss, Point(1024, 1024));
		ConvolutionResultPtr	c = ai * a * b;
		ImagePtr	imageptr = c->image();
		Image<double>	*imagep = dynamic_cast<Image<double>*>(&*imageptr);
		adapter::RollAdapter<double>	roll(*imagep, ImagePoint(1024, 1024));
		ImagePtr	rolled = ImagePtr(new Image<double>(roll));

		io::FITSout	out("tmp/uranus-gauss.fits");
		out.setPrecious(false);
		out.write(imageptr);
	}
	{
		ConvolutionResult	a(uranus, Point(1024, 1024));
		ConvolutionResult	b(herschel, Point(1024, 1024));
		ConvolutionResultPtr	c = ai * a * b;
		ImagePtr	imageptr = c->image();
		Image<double>	*imagep = dynamic_cast<Image<double>*>(&*imageptr);
		adapter::RollAdapter<double>	roll(*imagep, ImagePoint(1024, 1024));
		ImagePtr	rolled = ImagePtr(new Image<double>(roll));

		io::FITSout	out("tmp/uranus-herschel.fits");
		out.setPrecious(false);
		out.write(imageptr);
	}
		
#if 0
	DiskImage	uranus(ImageSize(2048,2048), ImagePoint(1024,1024),
				(M_PI / 180) * 4.1 / 3600, 6.5e-6 / 2.800);
	ImagePtr	uranusptr(new Image<double>(uranus));

	io::FITSout	uranusout("tmp/uranus.fits");
	uranusout.setPrecious(false);
	uranusout.write(uranusptr);
#endif
		
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testUranus() end");
}

#if 0
void	ConvolveTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
