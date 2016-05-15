/*
 * ImageTests.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>
#include <AstroIO.h>

using namespace astro::image;

namespace astro {
namespace test {

class ImageTest : public CppUnit::TestFixture {
private:
	Image<unsigned char>	*image;
public:
	void	setUp();
	void	tearDown();
	void	testByteImage();
	void	testCopyByteImage();
	void	testYUYVImage();
	void	testShortImage();
	void	testSubimage();
	void	testIterator();
	void	testScale();

	CPPUNIT_TEST_SUITE(ImageTest);
	CPPUNIT_TEST(testByteImage);
	CPPUNIT_TEST(testCopyByteImage);
	CPPUNIT_TEST(testYUYVImage);
	CPPUNIT_TEST(testShortImage);
	CPPUNIT_TEST(testSubimage);
	CPPUNIT_TEST(testIterator);
	CPPUNIT_TEST(testScale);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageTest);

void	ImageTest::setUp() {
	image = new Image<unsigned char>(640, 480);
	for (unsigned int i = 0; i < image->size().getPixels(); i++) {
		(*image)[i] = i % 160;
	}
}

void	ImageTest::tearDown() {
	delete image;
}

void	ImageTest::testByteImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testByteImage() begin");
	for (int x = 47; x < 100; x += 11) {
		for (int y = 18; y < 88; y += 13) {
			unsigned char	value = (x + y * 640) % 160;
			CPPUNIT_ASSERT(value == image->pixel(x, y));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testByteImage() end");
}

void	ImageTest::testCopyByteImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCopyByteImage() begin");
	Image<unsigned char>	image2 = *image;
	for (int x = 47; x < 100; x += 11) {
		for (int y = 18; y < 88; y += 13) {
			unsigned char	value = (x + y * 640) % 160;
			CPPUNIT_ASSERT(value == image2.pixel(x, y));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCopyByteImage() end");
}

void	ImageTest::testYUYVImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYUYVImage() begin");
	// test the conversion of an individual pixel
	YUYV<unsigned char>	p((unsigned char)47, 11);
	unsigned char	v;
	convertPixel(v, p);
	CPPUNIT_ASSERT(47 == v);

	// convert a complete image
	Image<YUYV<unsigned char> >	image2(640, 480);
	convertImage(image2, *image);
	CPPUNIT_ASSERT(image2.pixel(13, 15).y == (13 + 15 * 640) % 160);

	// convert to an unsigned short image
	Image<unsigned char>	image3(640, 480);
	convertImage(image3, image2);
	CPPUNIT_ASSERT(image3 == *image);
	image3.pixel(14,15) = 1;
	CPPUNIT_ASSERT(!(image3 == *image));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testYUYVImage() end");
}

void	ImageTest::testShortImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testShortImage() begin");
	Image<unsigned short>	image2(640, 480);
	convertImage(image2, *image);
	CPPUNIT_ASSERT(image2.pixel(13, 15) == ((13 + 15 * 640) % 160) * 256);
	Image<unsigned char>	image3(640,480);
	convertImage(image3, image2);
	CPPUNIT_ASSERT(image3 == *image);
	image3.pixel(14, 15) = 1;
	CPPUNIT_ASSERT(!(image3 == *image));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testShortImage() end");
}

void	ImageTest::testSubimage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSubimage() begin");
	ImageSize	size(10, 12);
	ImagePoint	origin(5, 9);
	ImageRectangle	frame(origin, size);
	Image<unsigned char>	image2(*image, frame);
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			unsigned char	v1 = image2.pixel(x, y);
			unsigned char	v2 = image->pixel(x + 5, y + 9);
			unsigned char	v3 = (5 + x + 640 * (9 + y)) % 160;
			CPPUNIT_ASSERT(v1 == v2);
			CPPUNIT_ASSERT(v2 == v3);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSubimage() end");
}

void	ImageTest::testIterator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() begin");
	Image<unsigned char>::row	row(*image, 7);
	Image<unsigned char>::iterator	i = row.begin();
	int	counter = 0;
	for (i = row.begin(); i != row.end(); i++) {
		unsigned char	v1 = image->pixel(counter, 7);
		unsigned char	v2 = *i;
		CPPUNIT_ASSERT(v1 == v2);
		counter++;
	}
	CPPUNIT_ASSERT(counter == 640);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testIterator() end");
}

void	ImageTest::testScale() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testScale() begin");
	ImageSize	size(256,256);
	Image<RGB<unsigned char> >	from(size);
	for (int x = 0; x < size.width(); x++) {
		for (int y = 0; y < size.height(); y++) {
			unsigned char	r = x;
			unsigned char	g = y;
			unsigned char	b = (x + y) / 2;
			RGB<unsigned char>	rgb(r, g, b);
			from.pixel(x, y) = rgb;
		}
	}

	io::FITSoutfile<RGB<unsigned char> >	out("tmp/rgb.fits");
	out.setPrecious(false);
	out.write(from);

	Image<RGB<unsigned char> >	*to
		= new Image<RGB<unsigned char> >(from, 0.125);

	io::FITSoutfile<RGB<unsigned char> >	scaledout("tmp/scaledrgb.fits");
	scaledout.setPrecious(false);
	scaledout.write(*to);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testScale() end");
}

} // namespace test
} // namespace astro
