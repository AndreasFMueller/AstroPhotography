/*
 * BrennerTest.cpp -- test the Brenner focus measure
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <math.h>
#include <AstroDebug.h>
#include <AstroFocus.h>
#include <AstroLoader.h>
#include <AstroIO.h>
#include <cstdlib>
#include <limits>

using namespace astro::focusing;

namespace astro {
namespace test {

#define N	45
#define stepsize	500

class BrennerTest : public CppUnit::TestFixture {
private:
	typedef std::pair<int, ImagePtr>	imagepair;
	typedef std::list<imagepair>		imagelist;
	imagelist	images;
	void	testCommon(FocusEvaluatorPtr evaluator);
public:
	void	setUp();
	void	tearDown();
	void	testHorizontal();
	void	testVertical();
	void	testOmni();

	CPPUNIT_TEST_SUITE(BrennerTest);
	CPPUNIT_TEST(testHorizontal);
	CPPUNIT_TEST(testVertical);
	CPPUNIT_TEST(testOmni);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(BrennerTest);

void	BrennerTest::setUp() {
	if (images.size() > 0) {
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up images for autofocus");
	// create a simulation camera 
	astro::module::Repository	repository;
	astro::module::Devices	devices(repository);
	camera::CameraPtr	camera
		= devices.getCamera(DeviceName("camera:simulator/camera"));
	camera::CcdPtr	ccd = camera->getCcd(0);
	camera::CoolerPtr	cooler = ccd->getCooler();
	cooler->setTemperature(-10);
	cooler->wait(10);
	camera::FocuserPtr	focuser
		= devices.getFocuser(DeviceName("focuser:simulator/focuser"));
	camera::FilterWheelPtr	filterwheel = camera->getFilterWheel();
	filterwheel->wait(60);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "camera ready");

	// prepare the exposure
	camera::Exposure	exposure;
	exposure.exposuretime(10);
	exposure.purpose(camera::Exposure::light);
	exposure.shutter(camera::Shutter::OPEN);

	// now compute the images
	int	center = (std::numeric_limits<unsigned short>::min()
			+ std::numeric_limits<unsigned short>::max()) / 2;
	unsigned short	first = center - (2 * N / 3) * stepsize;
	unsigned short	last = center + (N / 3) * stepsize;
	int	count = 0;
	for (unsigned short position = first; position <= last;
		position += stepsize) {
		ImagePtr	image;
		// first find out whether there already there is such 
		std::string	filename = stringprintf("tmp/brenner%05hu.fits",
			position);
		struct stat	sb;
		if (0 == stat(filename.c_str(), &sb)) {
			// read the existing file
			debug(LOG_DEBUG, DEBUG_LOG, 0, "read file %s",
				filename.c_str());
			io::FITSin	in(filename);
			image = in.read();
		} else {
			// move to this focus position
			focuser->moveto(position, 60);

			// get an exposure
			debug(LOG_DEBUG, DEBUG_LOG, 0, "exposing image %d",
				++count);
			ccd->startExposure(exposure);
			ccd->wait();
			image = ccd->getImage();

			// write the image to a file to able to
			io::FITSout	out(filename);
			out.setPrecious(false);
			out.write(image);
		}

		// add the image to the list of images
		images.push_back(imagepair(position, image));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "adding image at %d", position);
	}
}

void	BrennerTest::tearDown() {
}

void	BrennerTest::testCommon(FocusEvaluatorPtr evaluator) {
	FocusItems	focusitems;
	imagelist::const_iterator	i;
	for (i = images.begin(); i != images.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "evaluate image %s",
			i->second->size().toString().c_str());
		FocusItem	focusitem(i->first, (*evaluator)(i->second));
		focusitems.insert(focusitem);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d FocusItems",
		focusitems.size());

	for_each(focusitems.begin(), focusitems.end(),
		[](const FocusItem& focusitem) {
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"position = %d, value = %f",
				focusitem.position(), focusitem.value());
		}
	);

	// Solve using the parabolic solver
	ParabolicSolver	ps;
        int	p = ps.position(focusitems);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "error: %d", 
		p - std::numeric_limits<unsigned short>::max() / 2);
}

void	BrennerTest::testHorizontal() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testHorizontal() begin");
	FocusEvaluatorPtr	evaluator = FocusEvaluatorFactory::get(
				FocusEvaluatorFactory::BrennerHorizontal);
	testCommon(evaluator);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testHorizontal() end");
}

void	BrennerTest::testVertical() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testVertical() begin");
	FocusEvaluatorPtr	evaluator = FocusEvaluatorFactory::get(
				FocusEvaluatorFactory::BrennerVertical);
	testCommon(evaluator);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testVertical() end");
}

void	BrennerTest::testOmni() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOmni() begin");
	FocusEvaluatorPtr	evaluator = FocusEvaluatorFactory::get(
				FocusEvaluatorFactory::BrennerOmni);
	testCommon(evaluator);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testOmni() end");
}

} // namespace test
} // namespace astro
