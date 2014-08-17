/*
 * ProcessingControllerTest.cpp -- template for tests
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

class ControllerTestStep : public ProcessingStep {
	bool cancelrequest;
public:
	ControllerTestStep() { cancelrequest = false; }
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

class ProcessingControllerTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testNames();
	void	testExecute();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ProcessingControllerTest);
	CPPUNIT_TEST(testNames);
	CPPUNIT_TEST(testExecute);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProcessingControllerTest);

void	ProcessingControllerTest::setUp() {
}

void	ProcessingControllerTest::tearDown() {
}

void	ProcessingControllerTest::testNames() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNames() begin");
	ProcessingController	controller;
	ProcessingStepPtr	one =
		ProcessingStepPtr(new ControllerTestStep());
	controller.addstep("one", one);
	ProcessingStepPtr	two =
		ProcessingStepPtr(new ControllerTestStep());
	controller.addstep("two", two);
	ProcessingStepPtr	three =
		ProcessingStepPtr(new ControllerTestStep());
	controller.addstep("three", three);
	ProcessingStepPtr	four =
		ProcessingStepPtr(new ControllerTestStep());
	controller.addstep("four", four);
	CPPUNIT_ASSERT(&*controller.find("four") == &*four);
	CPPUNIT_ASSERT(&*controller.find("three") == &*three);
	CPPUNIT_ASSERT(&*controller.find("two") == &*two);
	CPPUNIT_ASSERT(&*controller.find("one") == &*one);
	CPPUNIT_ASSERT(controller.name(one) == std::string("one"));
	CPPUNIT_ASSERT(controller.name(two) == std::string("two"));
	CPPUNIT_ASSERT(controller.name(three) == std::string("three"));
	CPPUNIT_ASSERT(controller.name(four) == std::string("four"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testNames() end");
}

void	ProcessingControllerTest::testExecute() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testExecute() begin");
	ProcessingController	controller;
	ProcessingStepPtr	one =
		ProcessingStepPtr(new ControllerTestStep());
	controller.addstep("one", one);
	ProcessingStepPtr	two =
		ProcessingStepPtr(new ControllerTestStep());
	controller.addstep("two", two);
	ProcessingStepPtr	three =
		ProcessingStepPtr(new ControllerTestStep());
	controller.addstep("three", three);
	ProcessingStepPtr	four =
		ProcessingStepPtr(new ControllerTestStep());
	controller.addstep("four", four);
	controller.add_successor("one", "two");
	controller.add_successor("one", "three");
	controller.add_precursor("four", "two");
	controller.add_precursor("four", "three");
	controller.find("one")->checkstate();
	CPPUNIT_ASSERT(controller.find("one")->status()
		== ProcessingStep::needswork);
	controller.execute();
	CPPUNIT_ASSERT(controller.find("one")->status()
		== ProcessingStep::complete);
	CPPUNIT_ASSERT(controller.find("two")->status()
		== ProcessingStep::complete);
	CPPUNIT_ASSERT(controller.find("three")->status()
		== ProcessingStep::complete);
	CPPUNIT_ASSERT(controller.find("four")->status()
		== ProcessingStep::complete);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testExecute() end");
}

#if 0
void	ProcessingControllerTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
