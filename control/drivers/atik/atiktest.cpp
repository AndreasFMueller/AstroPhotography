/*
 * atiktest.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AtikLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

namespace astro {
namespace camera {
namespace atik {
namespace test {

class atiktest : public CppUnit::TestFixture {
	static AtikCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();
	CPPUNIT_TEST_SUITE(atiktest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST_SUITE_END();
};

AtikCameraLocator	*atiktest::locator = NULL;

CPPUNIT_TEST_SUITE_REGISTRATION(atiktest);

void	atiktest::setUp() {
	if (NULL == locator) {
		locator = new AtikCameraLocator();
	}
}

void	atiktest::tearDown() {
}

void	atiktest::testList() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testList() begin");
	std::vector<std::string>	cameras = locator->getDevicelist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera[%d]: %s", ++counter,
			i->c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d cameras found", counter);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testList() end");
}

} // namespace test
} // namespace atik
} // namespace camera
} // namespace astro
