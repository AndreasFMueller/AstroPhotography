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

using namespace astro::image;

namespace astro {
namespace test {

class ImageBufferTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testJPEG();
	void	testPNG();

	CPPUNIT_TEST_SUITE(ImageBufferTest);
	CPPUNIT_TEST(testJPEG);
	CPPUNIT_TEST(testPNG);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageBufferTest);

void	ImageBufferTest::setUp() {
}

void	ImageBufferTest::tearDown() {
}

void	ImageBufferTest::testJPEG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJPEG() begin");
	JPEG	jpeg;
	ImageBuffer	imagejpg("m57ok.jpg");
	CPPUNIT_ASSERT(imagejpg.buffersize() == 1201);
	ImageBuffer	*imagepng = imagejpg.convert(Format::PNG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "png size: %d", imagepng->buffersize());
	imagepng->write("m57ok.png");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testJPEG() end");
}

void	ImageBufferTest::testPNG() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPNG() begin");
	PNG	png;
	ImageBuffer	imagepng("t.png");
	CPPUNIT_ASSERT(imagepng.buffersize() == 2894830);
	ImageBuffer	*imagejpg = imagepng.convert(Format::JPEG);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "png size: %d", imagejpg->buffersize());
	imagejpg->write("t.jpg");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPNG() end");
}

} // namespace test
} // namespace astro
