/*
 * qsitest.cpp -- tests for the QSI driver
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <QsiLocator.h>
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
namespace qsi {
namespace test {

extern "C" double default_exposure;
extern "C" int default_imagecount;
extern "C" const char	*default_targetdirectory;
extern "C" const char	*default_prefix;

class qsitest : public CppUnit::TestFixture {
	static QsiCameraLocator	*locator;
public:
	void	setUp();
	void	tearDown();
	void	testList();

	CPPUNIT_TEST_SUITE(qsitest);
	CPPUNIT_TEST(testList);
	CPPUNIT_TEST_SUITE_END();
};

QsiCameraLocator	*qsitest::locator = NULL;

CPPUNIT_TEST_SUITE_REGISTRATION(qsitest);

void	qsitest::setUp() {
	if (NULL == locator) {
		locator = new QsiCameraLocator();
	}
}

void	qsitest::tearDown() {
}

void	qsitest::testList() {
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
} // namespace qsi
} // namespace camera
} // namespace astro
