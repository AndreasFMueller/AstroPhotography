/*
 * DeviceNameTest.cpp -- test the device name class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTypes.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace astro {
namespace test {

class DeviceNameTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testCast();
	void	testEquality();
	void	testCompare();

	CPPUNIT_TEST_SUITE(DeviceNameTest);
	CPPUNIT_TEST(testCast);
	CPPUNIT_TEST(testEquality);
	CPPUNIT_TEST(testCompare);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DeviceNameTest);

void	DeviceNameTest::setUp() {
}

void	DeviceNameTest::tearDown() {
}

void	DeviceNameTest::testCast() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCast() begin");
	DeviceName	name1("net", "blubb");
	std::string	stringname = (std::string)name1;
	CPPUNIT_ASSERT(stringname == std::string("net:blubb"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCast() end");
}

void	DeviceNameTest::testEquality() {
	DeviceName	name1("net", "blubb");
	DeviceName	name2("net:blubb");
	CPPUNIT_ASSERT(name1 == name2);
}

void	DeviceNameTest::testCompare() {
	DeviceName	nameA1("A", "1");
	DeviceName	nameA2("A", "2");
	DeviceName	nameB1("B", "1");
	DeviceName	nameB2("B", "2");
	CPPUNIT_ASSERT(nameA1 < nameA2);
	CPPUNIT_ASSERT(nameA1 < nameB1);
	CPPUNIT_ASSERT(nameA1 < nameB2);
	CPPUNIT_ASSERT(nameA2 < nameB1);
	CPPUNIT_ASSERT(nameA2 < nameB2);
	CPPUNIT_ASSERT(nameB1 < nameB2);

	CPPUNIT_ASSERT(!(nameA1 < nameA1));
	CPPUNIT_ASSERT(!(nameA2 < nameA1));
	CPPUNIT_ASSERT(!(nameB1 < nameA1));
	CPPUNIT_ASSERT(!(nameB2 < nameA1));

	CPPUNIT_ASSERT(!(nameA2 < nameA2));
	CPPUNIT_ASSERT(!(nameB1 < nameA2));
	CPPUNIT_ASSERT(!(nameB2 < nameA2));

	CPPUNIT_ASSERT(!(nameB1 < nameB1));
	CPPUNIT_ASSERT(!(nameB2 < nameB1));

	CPPUNIT_ASSERT(!(nameB2 < nameB2));
}

} // namespace test
} // namespace astro
