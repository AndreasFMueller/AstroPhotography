/*
 * WindowAdapterTest.cpp -- test adapters to subwindows
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <iostream>

using namespace astro::image;

namespace astro {
namespace test {

class WindowAdapterTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testWindowAdapter();

	CPPUNIT_TEST_SUITE(WindowAdapterTest);
	CPPUNIT_TEST(testWindowAdapter);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(WindowAdapterTest);

void	WindowAdapterTest::setUp() {
}

void	WindowAdapterTest::tearDown() {
}

void	WindowAdapterTest::testWindowAdapter() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window adapter test");
	// create an image
	Image<unsigned char>	image(16, 16);
	for (unsigned int x = 0; x < 16; x++) {
		for (unsigned int y = 0; y < 16; y++) {
			image.pixel(x, y) = x * y;
		}
	}

	// create the subframe
	ImageRectangle	frame(ImagePoint(4, 4), ImageSize(8, 8));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "frame: %s", frame.toString().c_str());

	// create an adapter for a subframe
	WindowAdapter<unsigned char>	adapter(image, frame);

	// access the subframe
	ImageSize	size = adapter.getSize();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adapter size: %s",
		size.toString().c_str());
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
			unsigned char	value = adapter.pixel(x, y);
			unsigned char	v
				= (frame.origin.x + x) * (frame.origin.y + y);
			if (v != value) {
				debug(LOG_DEBUG, DEBUG_LOG, 0, "expected %d != %d found",
					(int)v, (int)value);
			}
			CPPUNIT_ASSERT(value == v);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "window adapter test complete");
}

} // namespace test
} // namespace astro
