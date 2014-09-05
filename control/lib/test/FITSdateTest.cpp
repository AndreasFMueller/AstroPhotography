/*
 * FITSdateTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroImage.h>

using namespace astro::image;

namespace astro {
namespace test {

class FITSdateTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(FITSdateTest);
	CPPUNIT_TEST(testConstructor);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FITSdateTest);

void	FITSdateTest::setUp() {
}

void	FITSdateTest::tearDown() {
}

void	FITSdateTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");

	std::string	s1("1962-02-14");
	FITSdate	d1(s1);
	CPPUNIT_ASSERT(d1.showShort() == s1);

	std::string	s2("1999-04-18T12:13:14");
	FITSdate	d2(s2);
	CPPUNIT_ASSERT(d2.showLong() == s2);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

#if 0
void	FITSdateTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
