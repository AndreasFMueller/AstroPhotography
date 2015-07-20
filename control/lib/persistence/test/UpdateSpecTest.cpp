/*
 * UpdateSpecTest.cpp -- tests for the UpdateSpec class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroPersistence.h>
#include <math.h>

using namespace astro::persistence;

namespace astro {
namespace test {

class UpdateSpecTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();

	CPPUNIT_TEST_SUITE(UpdateSpecTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UpdateSpecTest);

void	UpdateSpecTest::setUp() {
}

void	UpdateSpecTest::tearDown() {
}

void	UpdateSpecTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	FieldValueFactory	factory;
	UpdateSpec	spec;
	spec.insert(Field("intfield", factory.get(47)));
	spec.insert(Field("floatfield", factory.get(4.7)));
	spec.insert(Field("stringfield", factory.get("siebenundvierzig")));
	spec.insert(Field("timefield", factory.getTime("2014-01-01 12:34:56")));

	std::string	uq = spec.updatequery("testtable");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update query: %s", uq.c_str());

	std::string	iq = spec.insertquery("testtable");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "insert query: %s", iq.c_str());

	std::string	sq = spec.selectquery("testtable");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "select query: %s", sq.c_str());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

} // namespace test
} // namespace astro
