/*
 * RawImageFileStepTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroProcess.h>
#include <AstroIO.h>

using namespace astro::image;
using namespace astro::process;
using namespace astro::io;

namespace astro {
namespace test {

class RawImageFileStepTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testFile();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(RawImageFileStepTest);
	CPPUNIT_TEST(testFile);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RawImageFileStepTest);

void	RawImageFileStepTest::setUp() {
}

void	RawImageFileStepTest::tearDown() {
}

void	RawImageFileStepTest::testFile() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFile() begin");
	RawImageFileStep	file("andromeda-base.fits");
	CPPUNIT_ASSERT(file.status() == ProcessingStep::needswork);
	file.work();
	CPPUNIT_ASSERT(file.status() == ProcessingStep::complete);
	ImageSize	size(3900, 2616);
	CPPUNIT_ASSERT(file.monochrome_preview().getSize() == size);
	CPPUNIT_ASSERT(file.out().getSize() == size);
	Image<unsigned char>	byteimage(file.monochrome_preview());
	FITSoutfile<unsigned char>	bytefile("andromeda-preview.fits");
	bytefile.setPrecious(false);
	bytefile.write(byteimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testFile() end");
}

#if 0
void	RawImageFileStepTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
