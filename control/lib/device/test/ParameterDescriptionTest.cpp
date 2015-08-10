/*
 * ParameterTest.cpp -- test the Parameter description stuff
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>

namespace astro {
namespace test {

class ParameterTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testBoolean();
	void	testRange();
	void	testSequence();
	void	testSetFloat();
	void	testSetString();
	void	testLargeSet();

	CPPUNIT_TEST_SUITE(ParameterTest);
	CPPUNIT_TEST(testBoolean);
	CPPUNIT_TEST(testRange);
	CPPUNIT_TEST(testSequence);
	CPPUNIT_TEST(testSetFloat);
	CPPUNIT_TEST(testSetString);
	CPPUNIT_TEST(testLargeSet);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ParameterTest);

void	ParameterTest::setUp() {
}

void	ParameterTest::tearDown() {
}

void	ParameterTest::testBoolean() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBoolean() begin");
	astro::device::ParameterDescription	desc("test");
	CPPUNIT_ASSERT(desc.name() == "test");
	CPPUNIT_ASSERT(desc.isvalid("true"));
	CPPUNIT_ASSERT(desc.isvalid("false"));
	CPPUNIT_ASSERT(!desc.isvalid("false "));
	CPPUNIT_ASSERT(!desc.isvalid(" true"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBoolean() end");
}

void	ParameterTest::testRange() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRange() begin");
	astro::device::ParameterDescription	desc("test", 2, 4.1);
	CPPUNIT_ASSERT(desc.name() == "test");
	CPPUNIT_ASSERT(desc.isvalid(2));
	CPPUNIT_ASSERT(desc.isvalid(2.000001));
	CPPUNIT_ASSERT(desc.isvalid(4.099999));
	CPPUNIT_ASSERT(desc.isvalid(4.1));
	CPPUNIT_ASSERT(!desc.isvalid(1.99999));
	CPPUNIT_ASSERT(!desc.isvalid(4.10001));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRange() end");
}

void	ParameterTest::testSequence() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSequence() begin");
	astro::device::ParameterDescription	desc("test", 1, 3, 0.1);
	CPPUNIT_ASSERT(desc.isvalid(2));
	CPPUNIT_ASSERT(!desc.isvalid(2.05));
	CPPUNIT_ASSERT(desc.isvalid(2.0005));
	CPPUNIT_ASSERT(desc.isvalid(2.001));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSequence() end");
}

void	ParameterTest::testSetFloat() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSetFloat() begin");
	astro::device::ParameterDescription	desc("test", std::set<float>());
	CPPUNIT_ASSERT(desc.name() =="test");
	CPPUNIT_ASSERT(!desc.isvalid(3.1415));
	desc.add(3.1415);
	CPPUNIT_ASSERT(desc.isvalid(3.1415));
	desc.add(1.4142);
	CPPUNIT_ASSERT(desc.isvalid(1.4142));
	CPPUNIT_ASSERT(desc.isvalid(3.1415));
	CPPUNIT_ASSERT(desc.isvalid(1.414200001));
	CPPUNIT_ASSERT(desc.isvalid(1.41420001));
	CPPUNIT_ASSERT(desc.isvalid(1.4142001));
	CPPUNIT_ASSERT(desc.isvalid(1.414201));
	CPPUNIT_ASSERT(desc.isvalid(1.4141));
	CPPUNIT_ASSERT(desc.isvalid(1.413));
	CPPUNIT_ASSERT(!desc.isvalid(1.42));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSetFloat() end");
}

void	ParameterTest::testSetString() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSetString() begin");
	astro::device::ParameterDescription	desc("test",
		std::set<std::string>());
	CPPUNIT_ASSERT(desc.name() == "test");
	CPPUNIT_ASSERT(!desc.isvalid("blubb"));
	desc.add("blubb");
	CPPUNIT_ASSERT(desc.isvalid("blubb"));
	desc.add("foo");
	CPPUNIT_ASSERT(desc.isvalid("foo"));
	CPPUNIT_ASSERT(desc.isvalid("blubb"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSetString() end");
}

void	ParameterTest::testLargeSet() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testLargeSet() begin");
	astro::device::ParameterDescription	desc("test",
		std::set<float>());
	for (int i = 0; i <= 10000; i++) {
		desc.add(sqrt(i));
	}
	CPPUNIT_ASSERT(desc.isvalid(sqrt(2)));
	CPPUNIT_ASSERT(!desc.isvalid(sqrt(2.5)));
	CPPUNIT_ASSERT(desc.isvalid(sqrt(10000)));
	CPPUNIT_ASSERT(!desc.isvalid(sqrt(10000.5)));
	CPPUNIT_ASSERT(!desc.isvalid(sqrt(9999.5)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testLargeSet() end");
}

/*
void	ParameterTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
*/

} // namespace test
} // namespace astro
