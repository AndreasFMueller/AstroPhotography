/*
 * WriteImageFileStepTest.cpp -- test the image file writer step
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroProcess.h>
#include <AstroIO.h>
#include <includes.h>

using namespace astro::io;
using namespace astro::process;
using namespace astro::image;

namespace astro {
namespace test {

class WriteImageFileStepTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testWrite();

	CPPUNIT_TEST_SUITE(WriteImageFileStepTest);
	CPPUNIT_TEST(testWrite);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(WriteImageFileStepTest);

void	WriteImageFileStepTest::setUp() {
}

void	WriteImageFileStepTest::tearDown() {
}

void	WriteImageFileStepTest::testWrite() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWrite() begin");
	unlink("andromeda-double.fits");
	ProcessingStep	*input = new RawImageFile("andromeda-base.fits");
	ProcessingStep	*output = new WriteImage("andromeda-double.fits");
	output->add_precursor(input);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "input step prepared");
	input->work();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "input step executed");
	CPPUNIT_ASSERT(input->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(output->status() == ProcessingStep::needswork);
	output->work();
	CPPUNIT_ASSERT(output->status() == ProcessingStep::complete);
	FITSin	in("andromeda-double.fits");
	ImagePtr	image = in.read();
	CPPUNIT_ASSERT(image->size() == ImageSize(3900, 2616));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWrite() end");
}

} // namespace test
} // namespace astro
