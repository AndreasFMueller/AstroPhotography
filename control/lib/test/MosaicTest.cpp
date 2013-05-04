/*
 * MosaicTests.cpp
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::image;

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

	CPPUNIT_TEST_SUITE(MosaicTest);
	CPPUNIT_TEST(testNone);
	CPPUNIT_TEST(testRGGB);
	CPPUNIT_TEST(testGRBG);
	CPPUNIT_TEST(testGBRG);
	CPPUNIT_TEST(testBGGR);
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
	for (int x = 0; x < image->size.width; x++) {
		for (int y = 0; y < image->size.height; y++) {
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
	for (int x = 0; x < image->size.width; x += 2) {
		for (int y = 0; y < image->size.height; y += 2) {
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
	for (int x = 0; x < image->size.width; x += 2) {
		for (int y = 0; y < image->size.height; y += 2) {
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
	for (int x = 0; x < image->size.width; x += 2) {
		for (int y = 0; y < image->size.height; y += 2) {
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
	for (int x = 0; x < image->size.width; x += 2) {
		for (int y = 0; y < image->size.height; y += 2) {
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

} // namespace test
} // namespace astro
