/*
 * ImageServerTest.cpp -- Tests for the ImageServer class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroProject.h>
#include <includes.h>

using namespace astro::project;
using namespace astro::persistence;

namespace astro {
namespace test {

class ImageServerTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testScan();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ImageServerTest);
	CPPUNIT_TEST(testScan);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageServerTest);

void	ImageServerTest::setUp() {
}

void	ImageServerTest::tearDown() {
}

void	ImageServerTest::testScan() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testScan() begin");
	// dummy database
	Database	database;
	
	char	path[MAXPATHLEN];
	std::string	directory(getcwd(path, MAXPATHLEN));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scan directory %s", directory.c_str());

	ImageServer	server(database, directory);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testScan() end");
}

#if 0
void	ImageServerTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
