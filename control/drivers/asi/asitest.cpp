/*
 * asitest.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AsiLocator.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>

namespace astro {
namespace camera {
namespace asi {
namespace test {

class asitest : public CppUnit::TestFixture {
	static AsiCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();
	CPPUNIT_TEST_SUITE(asitest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST_SUITE_END();
};

AsiCameraLocator	*asitest::locator = NULL;

CPPUNIT_TEST_SUITE_REGISTRATION(asitest);

void	asitest::setUp() {
	if (NULL == locator) {
		locator = new AsiCameraLocator();
	}
}

void	asitest::tearDown() {
}

void	asitest::testList() {
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
} // namespace asi
} // namespace camera
} // namespace astro
