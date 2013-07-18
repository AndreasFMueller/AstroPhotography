/*
 * WindowAdapterTest.cpp -- test adapters to subwindows
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <debug.h>
#include <iostream>

using namespace astro::image;

namespace astro {
namespace test {

class WindowAdapterTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testWindowAdapter();
	void	testConvertingWindowAdapter();

	CPPUNIT_TEST_SUITE(WindowAdapterTest);
	CPPUNIT_TEST(testWindowAdapter);
	CPPUNIT_TEST(testConvertingWindowAdapter);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(WindowAdapterTest);

void	WindowAdapterTest::setUp() {
}

void	WindowAdapterTest::tearDown() {
}

void	WindowAdapterTest::testWindowAdapter() {
	// create an image
	Image<unsigned char>	image(16, 116);
	for (unsigned int x = 0; x < 16; x++) {
		for (unsigned int y = 0; y < 16; y++) {
			image.pixel(x, y) = x * y;
		}
	}

	// create the subframe
	ImageRectangle	frame(ImagePoint(4, 4), ImageSize(8, 8));

	// create an adapter for a subframe
	WindowAdapter<unsigned char>	adapter(image, frame);

	// access the subframe
	ImageSize	size = adapter.getSize();
	for (unsigned int x = 0; x < size.width; x++) {
		for (unsigned int y = 0; y < size.height; y++) {
			unsigned char	value = adapter.pixel(x, y);
			unsigned char	v
				= (frame.origin.x + x) * (frame.origin.y + y);
			CPPUNIT_ASSERT(value == v);
		}
	}
}

void	WindowAdapterTest::testConvertingWindowAdapter() {
	// create an image
	Image<unsigned char>	image(16, 16);
	for (unsigned int x = 0; x < 16; x++) {
		for (unsigned int y = 0; y < 16; y++) {
			image.pixel(x, y) = x * y;
		}
	}

	// create the subframe
	ImageRectangle	frame(ImagePoint(4, 4), ImageSize(8, 8));

	// create an adapter for a subframe
	ConvertingWindowAdapter<double, unsigned char>	adapter(image, frame);

	// access the subframe
	ImageSize	size = adapter.getSize();
	for (unsigned int x = 0; x < size.width; x++) {
		for (unsigned int y = 0; y < size.height; y++) {
			double	value = adapter.pixel(x, y);
			double	v = (frame.origin.x + x) * (frame.origin.y + y);
			CPPUNIT_ASSERT(value == v);
		}
	}
}


} // namespace test
} // namespace astro
