/*
 * ConvertingAdapterTest.cpp -- test adapters to conversions
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

class ConvertingAdapterTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testConvertingAdapter();
	void	testConvertingConvertingAdapter();

	CPPUNIT_TEST_SUITE(ConvertingAdapterTest);
	CPPUNIT_TEST(testConvertingAdapter);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConvertingAdapterTest);

void	ConvertingAdapterTest::setUp() {
}

void	ConvertingAdapterTest::tearDown() {
}

void	ConvertingAdapterTest::testConvertingAdapter() {
	// create an image
	Image<unsigned char>	image(16, 16);
	for (unsigned int x = 0; x < 16; x++) {
		for (unsigned int y = 0; y < 16; y++) {
			image.pixel(x, y) = x * y;
		}
	}

	// create an adapter for a subframe
	ConvertingAdapter<float, unsigned char>	adapter(image);

	// access the subframe
	ImageSize	size = adapter.getSize();
	for (unsigned int x = 0; x < size.getWidth(); x++) {
		for (unsigned int y = 0; y < size.getHeight(); y++) {
			float	value = adapter.pixel(x, y);
			float	v = x * y;
			CPPUNIT_ASSERT(value == v);
		}
	}
}

} // namespace test
} // namespace astro
