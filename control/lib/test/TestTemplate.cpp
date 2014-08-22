/*
 * TestTemplate.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>

namespace astro {
namespace test {

class TestTemplate: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(TestTemplate);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestTemplate);

void	TestTemplate::setUp() {
}

void	TestTemplate::tearDown() {
}

#if 0
void	TestTemplate::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
