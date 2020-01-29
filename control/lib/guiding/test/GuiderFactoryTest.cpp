/*
 * GuiderFactoryTest.cpp -- tests for the GuiderFactory class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroGuiding.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <AstroDebug.h>

using namespace astro::module;
using namespace astro::guiding;

namespace astro {
namespace test {

class GuiderFactoryTest : public CppUnit::TestFixture {
	std::string	path;
public:
	void	setUp();
	void	tearDown();
	void	test();

	CPPUNIT_TEST_SUITE(GuiderFactoryTest);
	CPPUNIT_TEST(test);
	CPPUNIT_TEST_SUITE_END();
};

void	GuiderFactoryTest::setUp() {
}

void	GuiderFactoryTest::tearDown() {
}

void	GuiderFactoryTest::test() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test() begin");
	GuiderFactory	guiderfactory;
	GuiderDescriptor	guiderdescriptor("TEST",
		"ccd:simulator/Imaging", "guideport:simulator/guideport", "");
	GuiderPtr	guider = guiderfactory.get(guiderdescriptor);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "guider constructed");
	std::vector<GuiderDescriptor>	guiders = guiderfactory.list();
	CPPUNIT_ASSERT(guiders.size() == 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(GuiderFactoryTest);

} // namespace test
} // namespace astro
