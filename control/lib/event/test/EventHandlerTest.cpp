/*
 * EventHandlerTests.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroImage.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroConfig.h>
#include <AstroEvent.h>

using namespace astro::event;
using namespace astro::persistence;
using namespace astro::config;

namespace astro {
namespace test {

class EventHandlerTest : public CppUnit::TestFixture {
	Database	database;
public:
	void	setUp();
	void	tearDown();
	void	testEventHandler();

	CPPUNIT_TEST_SUITE(EventHandlerTest);
	CPPUNIT_TEST(testEventHandler);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(EventHandlerTest);

void	EventHandlerTest::setUp() {
	if (!database) {
		database = Configuration::get()->database();
	}
}

void	EventHandlerTest::tearDown() {
}

void	EventHandlerTest::testEventHandler() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEventHandler() begin");
	EventTable	table(database);
	table.remove("0 = 0");
	EventHandler::active(true);
	::event(EVENT_LOG, Event::DEBUG, "handler test");
	EventRecord	record = table.byid(1);
	CPPUNIT_ASSERT(record.pid == getpid());
	CPPUNIT_ASSERT(record.subsystem == "debug");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testEventHandler() end");
}

} // namespace test
} // namespace astro
