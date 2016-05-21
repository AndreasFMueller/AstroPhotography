/*
 * ConfigurationTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroConfig.h>

using namespace astro::config;

namespace astro {
namespace test {

class ConfigurationTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testConfiguration();
	void	testRecall();
	void	testRemove();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ConfigurationTest);
	CPPUNIT_TEST(testConfiguration);
	CPPUNIT_TEST(testRecall);
	CPPUNIT_TEST(testRemove);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ConfigurationTest);

void	ConfigurationTest::setUp() {
}

void	ConfigurationTest::tearDown() {
}

void	ConfigurationTest::testConfiguration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConfiguration() begin");
	ConfigurationPtr	configuration
		= Configuration::get("configtest.db");
	configuration->set("global", ".", "name1", "value1");
	configuration->set("global", ".", "name2", "value2");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConfiguration() end");
}

void	ConfigurationTest::testRecall() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRecall() begin");
	ConfigurationPtr	configuration
		= Configuration::get("configtest.db");
	CPPUNIT_ASSERT(configuration->get("global", ".", "name1") == "value1");
	CPPUNIT_ASSERT(configuration->get("global", ".", "name2") == "value2");
	CPPUNIT_ASSERT(configuration->get("global", ".", "name3", "value3")
		== "value3");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRecall() end");
}

void	ConfigurationTest::testRemove() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRemove() begin");
	ConfigurationPtr	configuration
		= Configuration::get("configtest.db");
	configuration->remove("global", ".", "name1");
	CPPUNIT_ASSERT(configuration->get("global", ".", "name1", "value3")
		== "value3");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRemove() end");
}

#if 0
void	ConfigurationTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
