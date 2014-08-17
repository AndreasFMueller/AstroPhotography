/*
 * ProcessingStepTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>

using namespace astro::image;
using namespace astro::process;

namespace astro {
namespace test {

class TestStep : public ProcessingStep {
	bool cancelrequest;
public:
	TestStep() { cancelrequest = false; }
	virtual ProcessingStep::state	do_work() {
#if 0
		cancelrequest = false;
		int	s = 20;
		while (s-- > 0) {
			_comletion = s / 20.
			usleep(100000)
			if (cancelrequest) {
				return;
			}
		}
#endif
		debug(LOG_DEBUG, DEBUG_LOG, 0, "working");
		return ProcessingStep::complete;
	}
	void	cancel() {
		cancelrequest = true;
	}
};

class ProcessingStepTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testBase();
	void	testDependency();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ProcessingStepTest);
	CPPUNIT_TEST(testBase);
	CPPUNIT_TEST(testDependency);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessingStepTest);

void	ProcessingStepTest::setUp() {
}

void	ProcessingStepTest::tearDown() {
}

void	ProcessingStepTest::testBase() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBase() begin");
	ProcessingStep	base;
	CPPUNIT_ASSERT(base.status() == ProcessingStep::idle);
	base.work();
	CPPUNIT_ASSERT(base.status() == ProcessingStep::idle);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting state needswork");
	base.status(ProcessingStep::needswork);
	CPPUNIT_ASSERT(base.status() == ProcessingStep::needswork);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "do work");
	base.work();
	CPPUNIT_ASSERT(base.status() == ProcessingStep::complete);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBase() end");
}

void	ProcessingStepTest::testDependency() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDependency() begin");
	ProcessingStepPtr	step1(new TestStep());
	ProcessingStepPtr	step2(new TestStep());
	ProcessingStepPtr	step3(new TestStep());
	ProcessingStepPtr	step4(new TestStep());
	step1->add_successor(step2);
	step1->add_successor(step3);
	step4->add_precursor(step2);
	step4->add_precursor(step3);
	step1->status(ProcessingStep::needswork);
	CPPUNIT_ASSERT(step2->status() == ProcessingStep::idle);
	CPPUNIT_ASSERT(step3->status() == ProcessingStep::idle);
	CPPUNIT_ASSERT(step4->status() == ProcessingStep::idle);
	step1->work();
	CPPUNIT_ASSERT(step1->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step2->status() == ProcessingStep::needswork);
	CPPUNIT_ASSERT(step3->status() == ProcessingStep::needswork);
	CPPUNIT_ASSERT(step4->status() == ProcessingStep::idle);
	step2->work();
	CPPUNIT_ASSERT(step1->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step2->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step3->status() == ProcessingStep::needswork);
	CPPUNIT_ASSERT(step4->status() == ProcessingStep::idle);
	step3->work();
	CPPUNIT_ASSERT(step1->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step2->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step3->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step4->status() == ProcessingStep::needswork);
	step4->work();
	CPPUNIT_ASSERT(step1->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step2->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step3->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(step4->status() == ProcessingStep::complete);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDependency() end");
}

#if 0
void	ProcessingStepTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
