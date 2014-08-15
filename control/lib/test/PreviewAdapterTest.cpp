/*
 * PreviewAdapterTest.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::adapter;

namespace astro {
namespace test {

class PreviewAdapterTest : public CppUnit::TestFixture {
private:
	Image<unsigned char>	*image;
public:
	void	setUp();
	void	tearDown();
	void	testShort();

	CPPUNIT_TEST_SUITE(PreviewAdapterTest);
	CPPUNIT_TEST(testShort);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PreviewAdapterTest);

void	PreviewAdapterTest::setUp() {
	image = new Image<unsigned char>(640, 480);
	for (unsigned int i = 0; i < image->size().getPixels(); i++) {
		(*image)[i] = i % 160;
	}
}

void	PreviewAdapterTest::tearDown() {
	delete image;
}

void	PreviewAdapterTest::testShort() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testShort() begin");
	Image<unsigned short>	*image2 = new Image<unsigned short>(64, 48);
	for (unsigned int i = 0; i < image2->size().getPixels(); i++) {
debug(LOG_DEBUG, DEBUG_LOG, 0, "set pixel %u to %u", i, i * 3);
		(*image2)[i] = i * 3;
	}
	ImagePtr	imageptr(image2);
	PreviewAdapter	*p = PreviewAdapter::get(imageptr);
	for (unsigned int y = 0; y < 48; y++) {
		for (unsigned int x = 0; x < 64; x++) {
			int	i = imageptr->pixeloffset(x, y);
			unsigned char	v = (3. * i) * 255. / 65535.;
			CPPUNIT_ASSERT(p->monochrome_pixel(x, y) == v);
		}
	}
	delete p;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testShort() end");
}

} // namespace test
} // namespace astro
