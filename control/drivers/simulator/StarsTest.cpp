/*
 * StarsTest.cpp -- test the simulated cooler
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Stars.h>
#include <SimUtil.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroImage.h>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace test {

class StarsTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testImage();

	CPPUNIT_TEST_SUITE(StarsTest);
	CPPUNIT_TEST(testImage);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(StarsTest);

void	StarsTest::setUp() {
}

void	StarsTest::tearDown() {
}

void	StarsTest::testImage() {
	ImageSize	size(640, 480);
	StarField	starfield(size, 100, 200);
	StarCamera<unsigned char>	starcamera(size);
	starcamera.noise(0.05);
	starcamera.addHotPixels(10);
	starcamera.light(true);
	ImagePtr	image = starcamera(starfield);
	FITSout	out("starfield.fits");
	out.setPrecious(false);
	out.write(image);
}

} // namespace test
} // namespace astro
