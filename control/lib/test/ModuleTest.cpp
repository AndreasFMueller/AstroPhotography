/*
 * ModuleTest.cpp -- tests for the Module class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */

#include <AstroLoader.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>

using namespace astro::module;

namespace astro {
namespace test {

class ModuleTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testBasic();

	CPPUNIT_TEST_SUITE(ModuleTest);
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST_SUITE_END();
};

void	ModuleTest::testBasic() {
	Module	module(".", "test_module");
	CPPUNIT_ASSERT(module.filename() == "./test_module.so");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ModuleTest);

} // namespace test
} // namespace astro
