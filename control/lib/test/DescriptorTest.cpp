/*
 * DescriptorTest.cpp -- tests for the Descriptor class
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

class DescriptorTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testBasic();

	CPPUNIT_TEST_SUITE(DescriptorTest);
	CPPUNIT_TEST(testBasic);
	CPPUNIT_TEST_SUITE_END();
};

void	DescriptorTest::testBasic() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() begin");
	Descriptor	d;
	CPPUNIT_ASSERT(d.name() == std::string(""));
	CPPUNIT_ASSERT(d.version() == std::string(VERSION));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBasic() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(DescriptorTest);

} // namespace test
} // namespace astro
