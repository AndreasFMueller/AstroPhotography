/*
 * OutlineTest.cpp -- test the outline stuff
 *
 *  (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroCatalog.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace astro {
namespace test {

class OutlineTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testOutline();

	CPPUNIT_TEST_SUITE(OutlineTest);
	CPPUNIT_TEST(testOutline);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(OutlineTest);

void	OutlineTest::setUp() {
}

void	OutlineTest::tearDown() {
}

void	OutlineTest::testOutline() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOutline() begin");
	catalog::OutlineCatalog	oc("/usr/local/starcatalogs/stellarium");
	//CPPUNIT_ASSERT(fabs(C.x()) < 1e-8);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOutline() end");
}

} // namespace test
} // namespace astro
