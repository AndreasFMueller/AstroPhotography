/*
 * RepositoryTest.cpp -- tests for the repository functions
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */

#include <AstroLoader.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>

using namespace astro::module;

namespace astro {
namespace test {

class RepositoryTest : public CppUnit::TestFixture {
	ModuleRepositoryPtr	repository;
	void	moduleTest(ModulePtr module);
public:
	void	setUp();
	void	tearDown();
	void	testListmodules();
	void	testOpen();
	void	testDescriptor();

	CPPUNIT_TEST_SUITE(RepositoryTest);
	CPPUNIT_TEST(testListmodules);
	CPPUNIT_TEST(testOpen);
	CPPUNIT_TEST(testDescriptor);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RepositoryTest);

void	RepositoryTest::setUp() {
	repository = getModuleRepository();
}

void	RepositoryTest::tearDown() {
	repository.reset();
}

void	RepositoryTest::testListmodules() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testListmodules() begin");
	const std::vector<ModulePtr>	modules
		= repository->modules();
	CPPUNIT_ASSERT(modules.size() >= 2);
	CPPUNIT_ASSERT(repository->contains("mock1"));
	CPPUNIT_ASSERT(repository->contains("mock2"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testListmodules() end");
}

void	RepositoryTest::testOpen() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOpen() begin");
	ModulePtr	module = repository->getModule("mock1");
	module->open();
	CPPUNIT_ASSERT(module->isloaded());
	module->close();
	CPPUNIT_ASSERT(!module->isloaded());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOpen() end");
}

void	RepositoryTest::moduleTest(ModulePtr module) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moduleTest() begin");
	ModuleDescriptor	*descriptor = module->getDescriptor();
	bool	result1 = descriptor->name() == "mock1";
	CPPUNIT_ASSERT(result1);
	bool	result2 = descriptor->version() == VERSION;
	CPPUNIT_ASSERT(result2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "moduleTest() end");
}

void	RepositoryTest::testDescriptor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDescriptor() begin");
	ModulePtr	module = repository->getModule("mock1");
	Module::dlclose_on_close = false;

	module->open();
	moduleTest(module);
	module->close();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDescriptor() end");
}

} // namespace test
} // namespace astro
