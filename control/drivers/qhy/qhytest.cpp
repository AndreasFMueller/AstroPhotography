/*
 * qhytest.cpp -- tests for the QHY driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QhyLocator.h>
#include <AstroIO.h>
#include <AstroFilter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <ostream>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroDemosaic.h>
#include <AstroFormat.h>

using namespace astro::image;
using namespace astro::image::filter;
using namespace astro::io;
using namespace astro::camera;

namespace astro {
namespace camera {
namespace qhy {
namespace test {

extern "C" double default_exposure;
extern "C" int default_imagecount;
extern "C" const char	*default_targetdirectory;
extern "C" const char	*default_prefix;

class qhytest : public CppUnit::TestFixture {
	static QhyCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();

	CPPUNIT_TEST_SUITE(qhytest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST_SUITE_END();
};

QhyCameraLocator	*qhytest::locator = NULL;

CPPUNIT_TEST_SUITE_REGISTRATION(qhytest);

void	qhytest::setUp() {
	if (NULL == locator) {
		locator = new QhyCameraLocator();
	}
}

void	qhytest::tearDown() {
}

void	qhytest::testList() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "devices");
	std::vector<std::string>	cameras = locator->getDevicelist();
	int	counter = 0;
	std::vector<std::string>::const_iterator	i;
	for (i = cameras.begin(); i != cameras.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "camera[%d]: %s", ++counter,
			i->c_str());
	}
}


} // namespace test
} // namespace qhy
} // namespace camera
} // namespace astro
