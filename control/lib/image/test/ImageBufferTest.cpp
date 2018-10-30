/*
 * ImageBufferTest.cpp -- test minimum radius function
 *
 * (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>
#include <math.h>
#include <sys/stat.h>

using namespace astro::image;

namespace astro {
namespace test {

class ImageBufferTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testJPEG();
	void	testJPEGmono();
	void	testPNG();
	void	testPNG16();

	CPPUNIT_TEST_SUITE(ImageBufferTest);
	CPPUNIT_TEST(testJPEG);
	CPPUNIT_TEST(testJPEGmono);
	CPPUNIT_TEST(testPNG);
	CPPUNIT_TEST(testPNG16);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageBufferTest);

void	ImageBufferTest::setUp() {
}

void	ImageBufferTest::tearDown() {
}

void	ImageBufferTest::testJPEG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJPEG() begin");
	std::string	filename("m57ok.jpg");
	struct stat	sb;
	stat(filename.c_str(), &sb);
	JPEG	jpeg;
	ImageBuffer	imagejpg(filename);
	CPPUNIT_ASSERT(imagejpg.buffersize() == sb.st_size);
	ImageBuffer	*imagepng = imagejpg.convert(Format::PNG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "png size: %d", imagepng->buffersize());
	imagepng->write("m57ok.png");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJPEG() end");
}

void	ImageBufferTest::testJPEGmono() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJPEGmono() begin");
	Image<unsigned char>	image(640, 480);
	image.fill(127);
	for (int x = 0; x < 640; x++) {
		for (int y = 0; y < 480; y++) {
			int	m = x * y;
			unsigned char	p = round(127. * (1 + sin(m / 2000.)));
			image.pixel(x, y) = p;
		}
	}
	JPEG	jpeg;
	jpeg.writeJPEG(image, "mono.jpg");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJPEGmono() end");
}

void	ImageBufferTest::testPNG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPNG() begin");
	std::string	filename("t.png");
	struct stat	sb;
	stat(filename.c_str(), &sb);
	PNG	png;
	ImageBuffer	imagepng(filename);
	CPPUNIT_ASSERT(imagepng.buffersize() == sb.st_size);
	ImagePtr	fitsimage = imagepng.image();
	JPEG	jpeg;
	jpeg.write(fitsimage, std::string("t0.jpg"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert to JPEG");
	ImageBuffer	*imagejpg = imagepng.convert(Format::JPEG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "png size: %d", imagejpg->buffersize());
	imagejpg->write("t.jpg");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPNG() end");
}

void	ImageBufferTest::testPNG16() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPNG16() begin");
	Image<unsigned short>	*image = new Image<unsigned short>(640, 480);
	image->fill(2047);
	for (int x = 0; x < 640; x++) {
		for (int y = 0; y < 480; y++) {
			int	m = x * y;
			unsigned char	p = round(2047. * (1 + sin(m / 2000.)));
			image->pixel(x, y) = p;
		}
	}
	PNG	png;
	ImagePtr	imageptr(image);
	png.writePNG(imageptr, "color16.png");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPNG16() end");
}


} // namespace test
} // namespace astro
