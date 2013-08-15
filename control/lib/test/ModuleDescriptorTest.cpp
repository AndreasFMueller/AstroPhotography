/*
 * ModuleDescriptorTest.cpp -- tests for the ModuleDescriptor class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <AstroDebug.h>

using namespace astro::module;

namespace astro {
namespace test {

class ModuleDescriptorTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testBasic();

	CPPUNIT_TEST_SUITE(ModuleDescriptorTest);
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST_SUITE_END();
};

void	ModuleDescriptorTest::testBasic() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() begin");
	ModuleDescriptor	d;
	CPPUNIT_ASSERT(d.name() == std::string(""));
	CPPUNIT_ASSERT(d.version() == std::string(VERSION));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ModuleDescriptorTest);

} // namespace test
} // namespace astro
