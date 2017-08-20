/*
 * ProcessorParserTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroProcess.h>
#include <includes.h>

using namespace astro::process;

namespace astro {
namespace test {

/**
 * \brief Test class for ProcessorParser
 */
class ProcessorParserTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testParse();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ProcessorParserTest);
	CPPUNIT_TEST(testParse);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessorParserTest);

void	ProcessorParserTest::setUp() {
}

void	ProcessorParserTest::tearDown() {
}

void	ProcessorParserTest::testParse() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testParse() begin");
	astro::process::ProcessorFactory	factory;
	ProcessorNetworkPtr	network = factory("process.xml");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testParse() end");
}

#if 0
void	ProcessorParserTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
