/*
 * PathTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroUtils.h>

namespace astro {
namespace test {

class PathTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testPath();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(PathTest);
	CPPUNIT_TEST(testPath);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PathTest);

void	PathTest::setUp() {
}

void	PathTest::tearDown() {
}

void	PathTest::testPath() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPath() begin");
	Path	path1("/usr/local/bin/test");
	CPPUNIT_ASSERT(path1.size() == 5);
	CPPUNIT_ASSERT(path1[0] == "");
	CPPUNIT_ASSERT(path1[1] == "usr");
	CPPUNIT_ASSERT(path1[2] == "local");
	CPPUNIT_ASSERT(path1[3] == "bin");
	CPPUNIT_ASSERT(path1[4] == "test");
	CPPUNIT_ASSERT(path1.basename() == "test");
	CPPUNIT_ASSERT(path1.dirname() == "/usr/local/bin");
	Path	path2("/usr/local/bin/");
	CPPUNIT_ASSERT(path2.size() == 4);
	CPPUNIT_ASSERT(path2[0] == "");
	CPPUNIT_ASSERT(path2[1] == "usr");
	CPPUNIT_ASSERT(path2[2] == "local");
	CPPUNIT_ASSERT(path2[3] == "bin");
	CPPUNIT_ASSERT(path2.basename() == "bin");
	CPPUNIT_ASSERT(path2.dirname() == "/usr/local");
	Path	path3("blubb/bla/foo");
	CPPUNIT_ASSERT(path3.size() == 3);
	CPPUNIT_ASSERT(path3[0] == "blubb");
	CPPUNIT_ASSERT(path3[1] == "bla");
	CPPUNIT_ASSERT(path3[2] == "foo");
	CPPUNIT_ASSERT(path3.basename() == "foo");
	CPPUNIT_ASSERT(path3.dirname() == "blubb/bla");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPath() end");
}

#if 0
void	PathTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
