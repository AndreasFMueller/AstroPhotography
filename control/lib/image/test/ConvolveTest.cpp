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
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ConvolveTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testConvolution);
	CPPUNIT_TEST(testInteger);
	CPPUNIT_TEST(testAbs);
	CPPUNIT_TEST(testPhase);
	CPPUNIT_TEST(testColor);
	CPPUNIT_TEST(testConvolutionResult);
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
	for (int x = 0; x < in.getSize().width(); x++) {
		for (int y = 0; y < in.getSize().height(); y++) {
			unsigned char	v = (x + y) % 256;
			in.pixel(x, y) = v;
			op.pixel(x, y) = 0;
		}
	}
	op.pixel(20, 30) = 1;
	ConvolutionResult	a(in, Point(0, 0));
	ConvolutionResult	b(op, Point(0, 0));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConvolutionResult() end");
}

#if 0
void	ConvolveTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
