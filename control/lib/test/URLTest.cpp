/*
 * URLTest.cpp -- test the URL class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace astro {
namespace test {

class URLTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testCast();
	void	testEncode();
	void	testDecode();

	CPPUNIT_TEST_SUITE(URLTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testCast);
	CPPUNIT_TEST(testEncode);
	CPPUNIT_TEST(testDecode);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(URLTest);

void	URLTest::setUp() {
}

void	URLTest::tearDown() {
}

void	URLTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	URL	url("method:bla1/bla2/bla3");
	CPPUNIT_ASSERT(url.size() == 3);
	CPPUNIT_ASSERT(url.method() == "method");
	CPPUNIT_ASSERT(url[0] == "bla1");
	CPPUNIT_ASSERT(url[1] == "bla2");
	CPPUNIT_ASSERT(url[2] == "bla3");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	URLTest::testCast() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCast() begin");
	std::string	s0("method:bla0/bla1/bla2");
	URL	url(s0);
	std::string	s = url;
	CPPUNIT_ASSERT(s == s0);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCast() end");
}

static std::string	plain("abcdefghijklmnopqrstuvwxyz0123456789/:%");
static std::string	encoded("abcdefghijklmnopqrstuvwxyz0123456789%2F%3A%25");

void	URLTest::testEncode() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEncode() begin");
	std::string	e = URL::encode(plain);
	CPPUNIT_ASSERT(e == encoded);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEncode() end");
}

void	URLTest::testDecode() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDecode() begin");
	std::string	d = URL::decode(encoded);
	CPPUNIT_ASSERT(d == plain);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDecode() end");
}

} // namespace test
} // namespace astro
