/*
 * Ucac4Test.cpp -- tests for the UCAC4 catalog 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Ucac4.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

using namespace astro::catalog;

namespace astro {
namespace test {

class Ucac4Test : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testNumber();
	void	testAccess();

	CPPUNIT_TEST_SUITE(Ucac4Test);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testNumber);
	CPPUNIT_TEST(testAccess);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ucac4Test);

void	Ucac4Test::setUp() {
}

void	Ucac4Test::tearDown() {
}

void	Ucac4Test::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	Ucac4	catalog("/usr/local/starcatalogs/u4");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	Ucac4Test::testNumber() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNumber() begin");
	Ucac4StarNumber	name1(47, 11);
	CPPUNIT_ASSERT(name1.toString() == std::string("UCAC4-047-000011"));
	Ucac4StarNumber	name2("UCAC4-047-000011");
	CPPUNIT_ASSERT(name1.toString() == name2.toString());
	CPPUNIT_ASSERT(name1 == name2);
	Ucac4StarNumber	name3("UCAC4-047-001100");
	CPPUNIT_ASSERT(name1 != name3);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNumber() end");
}

void	Ucac4Test::testAccess() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() begin");
	Ucac4	catalog("/usr/local/starcatalogs/u4");
	Ucac4StarNumber	name(47, 11);
	Ucac4Star	star = catalog.find(name);
	CPPUNIT_ASSERT(star.number == name);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testAccess() end");
}

} // namespace test
} // namespace astro
