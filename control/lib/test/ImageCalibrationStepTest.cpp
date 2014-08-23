/*
 * ImageCalibrationStepTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroProcess.h>
#include <AstroImage.h>

using namespace astro::image;
using namespace astro::process;

namespace astro {
namespace test {

class ImageCalibrationStepTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testCalibration();

	CPPUNIT_TEST_SUITE(ImageCalibrationStepTest);
	CPPUNIT_TEST(testCalibration);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageCalibrationStepTest);

void	ImageCalibrationStepTest::setUp() {
}

void	ImageCalibrationStepTest::tearDown() {
}

void	ImageCalibrationStepTest::testCalibration() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCalibration() begin");
	// create calibration images
	ImageSize	size(40, 32);
	Image<float>	*dark = new Image<float>(size);
	ImagePtr	darkptr(dark);
	Image<float>	*flat = new Image<float>(size);
	ImagePtr	flatptr(flat);
	Image<double>	*image = new Image<double>(size);
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; x < size.height(); y++) {
			dark->writablepixel(x, y);
			float	d = size.center().distance(ImagePoint(x, y));
			double	v = 1 + 0.1 * d;
			flat->writablepixel(x, y) = 1. / v;
			image->writablepixel(x, y) = v;
		}
	}

	// create calibration image steps
	CalibrationImageStep	*darkstep
		= new CalibrationImageStep(CalibrationImageStep::DARK, darkptr);
	ProcessingStepPtr	darkstepptr(darkstep);
	CalibrationImageStep	*flatstep
		= new CalibrationImageStep(CalibrationImageStep::FLAT, flatptr);
	ProcessingStepPtr	flatstepptr(flatstep);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testCalibration() end");
}

} // namespace test
} // namespace astro
