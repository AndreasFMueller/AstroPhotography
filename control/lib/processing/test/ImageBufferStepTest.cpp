/*
 * ImageBufferStepTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroProcess.h>

using namespace astro::image;
using namespace astro::process;

namespace astro {
namespace test {

class ImageBufferStepTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testBuffer();

	CPPUNIT_TEST_SUITE(ImageBufferStepTest);
	CPPUNIT_TEST(testBuffer);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageBufferStepTest);

void	ImageBufferStepTest::setUp() {
}

void	ImageBufferStepTest::tearDown() {
}

void	ImageBufferStepTest::testBuffer() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBuffer() begin");
	ImageSize	size(30, 20);
	ImageBufferStep	*buffer = new ImageBufferStep();
	ProcessingStepPtr	bufferstep(buffer);
	{
		// we do all the work in a special scope so that the
		// image will certainly have gone away when we compare the
		// images

		// create a test image
		Image<double>	*image = new Image<double>(size);
		ImagePtr	imageptr(image);
		for (unsigned int x = 0; x < size.width(); x++) {
			for (unsigned int y = 0; y < size.height(); y++) {
				image->writablepixel(x, y) = x * y;
			}
		}

		// create the processing pipeline
		RawImageStep	*raw = new RawImageStep(imageptr);
		ProcessingStepPtr	rawstep(raw);
		buffer->add_precursor(raw);

		// start working
		CPPUNIT_ASSERT(raw->status() == ProcessingStep::needswork);
		raw->work();
		CPPUNIT_ASSERT(raw->status() == ProcessingStep::complete);
		
		// do the work
		CPPUNIT_ASSERT(buffer->status() == ProcessingStep::needswork);
		bufferstep->work();
		CPPUNIT_ASSERT(buffer->status() == ProcessingStep::complete);
	}
	// now the image is gone again, but the contens should still be
	// the same
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			CPPUNIT_ASSERT(x * y == buffer->out().pixel(x, y));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBuffer() end");
}

} // namespace test
} // namespace astro
