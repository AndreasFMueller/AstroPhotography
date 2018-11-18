/*
 * SyncQueueTest.cpp -- Test the stacktrace handler
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroUtils.h>
#include <unistd.h>

namespace astro {
namespace test {

class SyncQueueTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testSyncQueue();

	CPPUNIT_TEST_SUITE(SyncQueueTest);
	CPPUNIT_TEST(testSyncQueue);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SyncQueueTest);

void	SyncQueueTest::setUp() {
}

void	SyncQueueTest::tearDown() {
}

void	produce(thread::SyncQueue<int> *queue) {
	int	i = 0;
	while (i < 15) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "pushing %d", i);
		queue->put(i++);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "pushing %d", i);
		queue->put(i++);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "pushing %d", i);
		queue->put(i++);
		::sleep(1);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "queue terminated");
	queue->terminate();
}

void	consume(thread::SyncQueue<int> *queue) {
	try {
		while (1) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "get next");
			int	i = queue->get();
			debug(LOG_DEBUG, DEBUG_LOG, 0, "received %d", i);
			::sleep(2);
		}
	} catch (std::range_error x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "queue completed");
	} catch (std::exception x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "exception: %s", x.what());
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "unknown exception");
	}
}

void	SyncQueueTest::testSyncQueue() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSyncQueue() begin");
	thread::SyncQueue<int>	q;
	std::thread	c(consume, &q);
	std::thread	p(produce, &q);
	c.join();
	p.join();
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSyncQueue() end");
}

} // namespace test
} // namespace astro
