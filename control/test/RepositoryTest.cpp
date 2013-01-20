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
	Repository	*repository;
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
	repository = new Repository();
}

void	RepositoryTest::tearDown() {
	delete repository;
}

void	RepositoryTest::testListmodules() {
	const std::vector<ModulePtr>	modules
		= repository->modules();
	CPPUNIT_ASSERT(modules.size() >= 2);
	CPPUNIT_ASSERT(repository->contains("mock1"));
	CPPUNIT_ASSERT(repository->contains("mock2"));
}

void	RepositoryTest::testOpen() {
	ModulePtr	module = repository->getModule("mock1");
	module->open();
	CPPUNIT_ASSERT(module->isloaded());
	module->close();
	CPPUNIT_ASSERT(!module->isloaded());
}

void	RepositoryTest::moduleTest(ModulePtr module) {
	DescriptorPtr	descriptor = module->getDescriptor();
	bool	result1 = descriptor->name() == "mock1";
	CPPUNIT_ASSERT(result1);
	bool	result2 = descriptor->version() == VERSION;
	CPPUNIT_ASSERT(result2);
}

void	RepositoryTest::testDescriptor() {

	ModulePtr	module = repository->getModule("mock1");
	Module::dlclose_on_close = false;

	module->open();
	moduleTest(module);
	module->close();
}

} // namespace test
} // namespace astro
