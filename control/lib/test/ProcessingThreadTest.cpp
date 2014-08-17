/*
 * ProcessingThreadTest.cpp -- test to verify workings of the ProcessingThread
 *                             class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroDebug.h>
#include <includes.h>

using namespace astro::process;

namespace astro {
namespace test {

class ThreadTestStep : public ProcessingStep {
	bool cancelrequest;
public:
	ThreadTestStep() { cancelrequest = false; }
	virtual ProcessingStep::state	do_work() {
		cancelrequest = false;
		int	s = 20;
		while (s-- > 0) {
			_completion = s / 20.;
			usleep(100000);
			if (cancelrequest) {
				return ProcessingStep::needswork;
			}
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "working");
		return ProcessingStep::complete;
	}
	void	cancel() {
		cancelrequest = true;
	}
};

class ProcessingThreadTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testWork();
	void	testCancel();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ProcessingThreadTest);
	CPPUNIT_TEST(testWork);
	CPPUNIT_TEST(testCancel);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessingThreadTest);

void	ProcessingThreadTest::setUp() {
}

void	ProcessingThreadTest::tearDown() {
}

void	ProcessingThreadTest::testWork() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWork() begin");
	ProcessingStepPtr	step(new ThreadTestStep());
	step->checkstate();
	CPPUNIT_ASSERT(step->status() == ProcessingStep::needswork);
	ProcessingThreadPtr	thread = ProcessingThread::get(step);
	thread->run();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current step state: %s",
		ProcessingStep::statename(step->status()).c_str());
	CPPUNIT_ASSERT(step->status() == ProcessingStep::working);
	thread->wait();
	CPPUNIT_ASSERT(step->status() == ProcessingStep::complete);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWork() end");
}

void	ProcessingThreadTest::testCancel() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCancel() begin");
	ProcessingStepPtr	step(new ThreadTestStep());
	step->checkstate();
	CPPUNIT_ASSERT(step->status() == ProcessingStep::needswork);
	ProcessingThreadPtr	thread = ProcessingThread::get(step);
	thread->run();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "current step state: %s",
		ProcessingStep::statename(step->status()).c_str());
	CPPUNIT_ASSERT(step->status() == ProcessingStep::working);
	sleep(1);
	thread->cancel();
	thread->wait();
	CPPUNIT_ASSERT(step->status() == ProcessingStep::needswork);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCancel() end");
}

#if 0
void	ProcessingThreadTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
