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
#include <AstroIO.h>
#include <math.h>

using namespace astro::process;
using namespace astro::io;

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
	ProcessingStepPtr	darkstep = ProcessingStepPtr(darkprocessor);
	controller.addstep("dark", darkstep);
	darkprocessor->tolerance(3.);

	FlatProcessorStep	*flatprocessor = new FlatProcessorStep();
	ProcessingStepPtr	flatstep = ProcessingStepPtr(flatprocessor);
	controller.addstep("flat", flatstep);

	ImageSize	size(512, 256);

	// create a number of raw images
	int	counter = 8;
	while (counter--) {
		std::string	name = stringprintf("raw%d", counter);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image named '%s'", name.c_str());
		Image<unsigned short>	*image
			= new Image<unsigned short>(size);
		darkimage(image);
		image->setMetadata(FITSKeywords::meta("EXPTIME", (double)47));
		image->setMetadata(FITSKeywords::meta("CCD-TEMP", (double)-20.3));
		image->setMetadata(FITSKeywords::meta("SET-TEMP", (double)-20.));
		image->setMetadata(FITSKeywords::meta("XBINNING", (long)2));
		image->setMetadata(FITSKeywords::meta("YBINNING", (long)2));
		image->setMetadata(FITSKeywords::meta("XORGSUBF", (long)23));
		image->setMetadata(FITSKeywords::meta("YORGSUBF", (long)32));

		// verify that the metadata is present
		CPPUNIT_ASSERT((double)(image->getMetadata("EXPTIME")) == (double)47);
		CPPUNIT_ASSERT((double)(image->getMetadata("CCD-TEMP")) == (double)-20.3);
		CPPUNIT_ASSERT((double)(image->getMetadata("SET-TEMP")) == (double)-20.);
		CPPUNIT_ASSERT((long)(image->getMetadata("XBINNING")) == (long)2);
		CPPUNIT_ASSERT((long)(image->getMetadata("YBINNING")) == (long)2);
		CPPUNIT_ASSERT((long)(image->getMetadata("XORGSUBF")) == (long)23);
		CPPUNIT_ASSERT((long)(image->getMetadata("YORGSUBF")) == (long)32);

		ImagePtr	imageptr(image);
		ProcessingStepPtr	processingstep =
			ProcessingStepPtr(new RawImageStep(imageptr));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "work on image '%s'", name.c_str());
		CPPUNIT_ASSERT(processingstep->status() == ProcessingStep::needswork);
		processingstep->work();
		CPPUNIT_ASSERT(processingstep->status() == ProcessingStep::complete);
		controller.addstep(name, processingstep);
		controller.add_precursor("dark", name);
		controller.add_precursor("flat", name);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "all raw images prepared");

	// ensure that the calibration processors know whether work needs to
	// be done
	darkprocessor->checkstate();
	flatprocessor->checkstate();
	CPPUNIT_ASSERT(darkprocessor->status() == ProcessingStep::needswork);
	CPPUNIT_ASSERT(flatprocessor->status() == ProcessingStep::needswork);

	// process the images
	controller.execute(1);
	//darkprocessor->work();
	CPPUNIT_ASSERT(darkprocessor->status() == ProcessingStep::complete);
	CPPUNIT_ASSERT(flatprocessor->status() == ProcessingStep::complete);

	// check the dark output adapter for bad pixels 
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking bad pixels in dark image");
	const ConstImageAdapter<double>&	darkout = darkprocessor->out();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "dark value(100, 200) = %f", darkout.pixel(100, 200));
	CPPUNIT_ASSERT(darkout.pixel(100, 200) != darkout.pixel(100, 200));
	CPPUNIT_ASSERT(darkout.pixel(110, 200) != darkout.pixel(110, 200));
	CPPUNIT_ASSERT(darkout.pixel(112, 200) != darkout.pixel(112, 200));
	CPPUNIT_ASSERT(darkout.pixel(400, 100) != darkout.pixel(400, 100));
	CPPUNIT_ASSERT(darkout.pixel(401, 100) != darkout.pixel(401, 100));

	CPPUNIT_ASSERT(fabs((double)darkprocessor->getMetadata("EXPTIME") - 47.) < 0.1);
	CPPUNIT_ASSERT(fabs((double)darkprocessor->getMetadata("CCD-TEMP") + 20.3) < 0.1);
	CPPUNIT_ASSERT(fabs((double)darkprocessor->getMetadata("SET-TEMP") + 20.) < 0.1);
	CPPUNIT_ASSERT((long)darkprocessor->getMetadata("XBINNING") == 2);
	CPPUNIT_ASSERT((long)darkprocessor->getMetadata("YBINNING") == 2);
	CPPUNIT_ASSERT((long)darkprocessor->getMetadata("XORGSUBF") == 23);
	CPPUNIT_ASSERT((long)darkprocessor->getMetadata("YORGSUBF") == 32);

	// count the values that are close to the target value
	int	goodpixels = 0;
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			double 	v = darkout.pixel(x, y);
			if (v == v) {
				if (fabs(v - (1000 + x + y)) < 100) {
					goodpixels++;
				}
			}
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "number of good pixels: %d", goodpixels);
	CPPUNIT_ASSERT(goodpixels > 0.8 * size.getPixels());

	// check the flat output adapter for bad pixels
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking bad pixels in flat image");
	const ConstImageAdapter<double>&	flatout = flatprocessor->out();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "flat value(100, 200) = %f", flatout.pixel(100, 200));
	CPPUNIT_ASSERT(flatout.pixel(100, 200) != flatout.pixel(100, 200));
	CPPUNIT_ASSERT(flatout.pixel(110, 200) != flatout.pixel(110, 200));
	CPPUNIT_ASSERT(flatout.pixel(112, 200) != flatout.pixel(112, 200));
	CPPUNIT_ASSERT(flatout.pixel(400, 100) != flatout.pixel(400, 100));
	CPPUNIT_ASSERT(flatout.pixel(401, 100) != flatout.pixel(401, 100));

	CPPUNIT_ASSERT(fabs((double)flatprocessor->getMetadata("CCD-TEMP") + 20.3) < 0.1);
	CPPUNIT_ASSERT(fabs((double)flatprocessor->getMetadata("SET-TEMP") + 20.) < 0.1);
	CPPUNIT_ASSERT((long)flatprocessor->getMetadata("XBINNING") == 2);
	CPPUNIT_ASSERT((long)flatprocessor->getMetadata("YBINNING") == 2);
	CPPUNIT_ASSERT((long)flatprocessor->getMetadata("XORGSUBF") == 23);
	CPPUNIT_ASSERT((long)flatprocessor->getMetadata("YORGSUBF") == 32);

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
