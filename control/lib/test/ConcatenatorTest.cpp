/*
 * ConcatenatorTest.cpp -- test the device name class
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

class ConcatenatorTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testCast();
	void	testOperator();
	void	testVector();
	void	testSet();

	CPPUNIT_TEST_SUITE(ConcatenatorTest);
	CPPUNIT_TEST(testCast);
	CPPUNIT_TEST(testOperator);
	CPPUNIT_TEST(testVector);
	CPPUNIT_TEST(testSet);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConcatenatorTest);

void	ConcatenatorTest::setUp() {
}

void	ConcatenatorTest::tearDown() {
}

void	ConcatenatorTest::testCast() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCast() begin");
	Concatenator	c(":");
	c(std::string("a"));
	c(std::string("b"));
	c(std::string("c"));
	std::string	result = c;
	CPPUNIT_ASSERT(std::string("a:b:c") == result);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCast() end");
}

void	ConcatenatorTest::testOperator() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOperator() begin");
	Concatenator	c("/");
	CPPUNIT_ASSERT(0 == c.componentcount());
	std::string	result = c;
	CPPUNIT_ASSERT(result == std::string(""));
	c(std::string("a"));
	result = c;
	CPPUNIT_ASSERT(1 == c.componentcount());
	CPPUNIT_ASSERT(result == std::string("a"));
	c(std::string("b"));
	result = c;
	CPPUNIT_ASSERT(result == std::string("a/b"));
	CPPUNIT_ASSERT(2 == c.componentcount());
	c(std::string("c"));
	result = c;
	CPPUNIT_ASSERT(result == std::string("a/b/c"));
	CPPUNIT_ASSERT(3 == c.componentcount());
	c(std::string("d"));
	result = c;
	CPPUNIT_ASSERT(result == std::string("a/b/c/d"));
	CPPUNIT_ASSERT(4 == c.componentcount());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOperator() end");
}

void	ConcatenatorTest::testVector() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testVector() begin");
	std::vector<std::string>	data;
	data.push_back(std::string("A"));
	data.push_back(std::string("B"));
	data.push_back(std::string("C"));
	std::string	result = Concatenator::concat(data, "/");
	CPPUNIT_ASSERT(result == std::string("A/B/C"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testVector() end");
}

void	ConcatenatorTest::testSet() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSet() begin");
	std::set<std::string>	data;
	data.insert(std::string("0"));
	data.insert(std::string("1"));
	data.insert(std::string("2"));
	std::string	result = Concatenator::concat(data, ", ");
	CPPUNIT_ASSERT(result == std::string("0, 1, 2"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSet() end");
}

} // namespace test
} // namespace astro
