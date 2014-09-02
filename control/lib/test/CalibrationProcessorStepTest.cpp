/*
 * CalibrationProcessorStepTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroProcess.h>
#include <AstroFormat.h>
#include <math.h>

using namespace astro::process;

namespace astro {
namespace test {

class CalibrationProcessorStepTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testDark();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(CalibrationProcessorStepTest);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST(testDark);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CalibrationProcessorStepTest);

void	CalibrationProcessorStepTest::setUp() {
}

void	CalibrationProcessorStepTest::tearDown() {
}

static double	normal_error(double sigma) {
	double	v = (random() - 1073741824.) / 2147483647.;
	if (v < -1) { v = -1; }
	if (v >  1) { v =  1; }
	double	x = v;
	double	xnew = v;
	double	delta;
	int	counter = 0;
	do {
		xnew = x - (erf(x) - v) * sqrt(M_PI) / (2 * exp(-x * x));
		delta = fabs(x - xnew);
		x = xnew;
		counter++;
	} while ((delta > 0.0001) && (counter < 30));
//	debug(LOG_DEBUG, DEBUG_LOG, 0, "v = %f -> x = %f (%d)", v, sigma * x, counter);
	return x * sigma;
}

static void	darkimage(Image<unsigned short>	*image) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filling image at %p", image);
	ImageSize	size = image->size();
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			unsigned short	v = 1000 + (x + y);
			image->writablepixel(x, y) = v;
		}
	}
	image->writablepixel(100, 200) = 65000;
	image->writablepixel(110, 200) = 65000;
	image->writablepixel(112, 200) = 65000;
	image->writablepixel(400, 100) = 65000;
	image->writablepixel(401, 100) = 65000;
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			unsigned short	v = image->pixel(x, y);
			v = v + normal_error(500);
//debug(LOG_DEBUG, DEBUG_LOG, 0, "v=%hu", v);
			image->writablepixel(x, y) = v;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filling image at %p", image);
}

void	CalibrationProcessorStepTest::testDark() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDark() begin");

	ProcessingController	controller;
	
	DarkProcessorStep	*darkprocessor = new DarkProcessorStep();
	ProcessingStepPtr	step = ProcessingStepPtr(darkprocessor);

	controller.addstep("dark", step);

	ImageSize	size(512, 256);

	// create a number of raw images
	int	counter = 8;
	while (counter--) {
		std::string	name = stringprintf("raw%d", counter);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image named '%s'", name.c_str());
		Image<unsigned short>	*image
			= new Image<unsigned short>(size);
		darkimage(image);

		ImagePtr	imageptr(image);
		ProcessingStepPtr	processingstep =
			ProcessingStepPtr(new RawImageStep(imageptr));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "work on image '%s'", name.c_str());
		CPPUNIT_ASSERT(processingstep->status() == ProcessingStep::needswork);
		processingstep->work();
		CPPUNIT_ASSERT(processingstep->status() == ProcessingStep::complete);
		controller.addstep(name, processingstep);
		controller.add_precursor("dark", name);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "all raw images prepared");

	// process the images
	//controller.execute(2);
	darkprocessor->checkstate();
	CPPUNIT_ASSERT(darkprocessor->status() == ProcessingStep::needswork);
	darkprocessor->work();
	CPPUNIT_ASSERT(darkprocessor->status() == ProcessingStep::complete);

	// check the output adapter for bad pixels 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking bad pixels");
	const ConstImageAdapter<double>&	out = darkprocessor->out();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dark value(100, 200) = %f", out.pixel(100, 200));
	CPPUNIT_ASSERT(out.pixel(100, 200) != out.pixel(100, 200));
	CPPUNIT_ASSERT(out.pixel(110, 200) != out.pixel(110, 200));
	CPPUNIT_ASSERT(out.pixel(112, 200) != out.pixel(112, 200));
	CPPUNIT_ASSERT(out.pixel(400, 100) != out.pixel(400, 100));
	CPPUNIT_ASSERT(out.pixel(401, 100) != out.pixel(401, 100));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDark() end");
}

#if 0
void	CalibrationProcessorStepTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
