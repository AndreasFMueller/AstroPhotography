/*
 * PhaseCorrelatorTest.cpp -- test adapters to subwindows
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroTransform.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroAdapter.h>
#include <iostream>
#include <sstream>
#include <includes.h>
#include <AstroCoordinates.h>

using namespace astro::image;
using namespace astro::image::transform;
using namespace astro::io;
using namespace astro::adapter;

namespace astro {
namespace test {

class PhaseCorrelatorTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testInteger();
	void	testIntegerNegative();
	void	testHalf();
	void	testImage();
	void	testSun();
	void	testJupiter();
	void	testJupiterSequence();
	void	testDisks();
	void	testTriangle();
	void	testMoon();
	void	testOrion();

	CPPUNIT_TEST_SUITE(PhaseCorrelatorTest);
//	CPPUNIT_TEST(testInteger);
//	CPPUNIT_TEST(testIntegerNegative);
//	CPPUNIT_TEST(testHalf);
//	CPPUNIT_TEST(testImage);
//	CPPUNIT_TEST(testSun);
//	CPPUNIT_TEST(testJupiter);
//	CPPUNIT_TEST(testJupiterSequence);
//	CPPUNIT_TEST(testDisks);
//	CPPUNIT_TEST(testTriangle);
//	CPPUNIT_TEST(testMoon);
	CPPUNIT_TEST(testOrion);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PhaseCorrelatorTest);

void	PhaseCorrelatorTest::setUp() {
}

void	PhaseCorrelatorTest::tearDown() {
}

void	PhaseCorrelatorTest::testInteger() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start Integer test");
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * (1 + cos(r / 10)) / r;
			}
			r = hypot(x - 63, y - 74);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * (1 + cos(r / 10)) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage).first;

	// display result
	std::ostringstream	out;
	out << translation;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s, should be (3,4)", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end Integer test");
}

void	PhaseCorrelatorTest::testIntegerNegative() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start IntegerNegative test");
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * (1 + cos(r / 10)) / r;
			}
			r = hypot(x - 57, y - 66);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * (1 + cos(r / 10)) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage).first;

	// display result
	std::ostringstream	out;
	out << translation;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s, should be (-3,-4)", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end IntegerNegative test");
}

void	PhaseCorrelatorTest::testHalf() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "start Half test");
	// create an image
	int	N = 256;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			double	r = hypot(x - 60, y - 70);
			if (r == 0) {
				fromimage.pixel(x, y) = 100;
			} else {
				fromimage.pixel(x, y) = 100 * (1 + cos(r / 8)) / r;
			}
			r = hypot(x - 62.5, y - 66.5);
			if (r == 0) {
				toimage.pixel(x, y) = 100;
			} else {
				toimage.pixel(x, y) = 100 * (1 + cos(r / 8)) / r;
			}
		}
	}

	// create a phase correclator
	PhaseCorrelator	pc;
	Point	translation = pc(fromimage, toimage).first;

	// display result
	std::ostringstream	out;
	out << translation;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "translation = %s, should be (2.5,-0.5)", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end Half test");
}

void	PhaseCorrelatorTest::testImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "beginImage test");

	FITSin	imagefile("testimages/test-image.fits");
	ImagePtr	imageptr = imagefile.read();
	Image<unsigned char>	*image = dynamic_cast<Image<unsigned char> *>(&*imageptr);
	TypeReductionAdapter<double, unsigned char>	doubleimage(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test image read");

	FITSin	chartfile("testimages/test-chart.fits");
	ImagePtr	chartptr = chartfile.read();
	Image<unsigned char>	*chart = dynamic_cast<Image<unsigned char> *>(&*chartptr);
	TypeReductionAdapter<double, unsigned char>	doublechart(*chart);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test chart read");

	// create a phase correlator
	PhaseCorrelator	pc(false);
	std::pair<Point, double>	result = pc(doubleimage, doublechart);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s, weight = %f",
		result.first.toString().c_str(), result.second);

	// expected result: (-15,26)
	Point	target(-15, 26);
	Point	effective(round(result.first.x()), round(result.first.y()));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s ?= %s",
		target.toString().c_str(), effective.toString().c_str());
	CPPUNIT_ASSERT(target == effective);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "endImage test");
}

void	PhaseCorrelatorTest::testSun() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin testSun()");

	FITSin	imagefile("testimages/sun.fits");
	ImagePtr	imageptr = imagefile.read();
	Image<RGB<unsigned char> >	*image
		= dynamic_cast<Image<RGB<unsigned char> > *>(&*imageptr);
	LuminanceAdapter<RGB<unsigned char>, double>	doubleimage(*image);
	DerivativeNormAdapter<double>	i1(doubleimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test image read");

	FITSin	translatedfile("testimages/sun-translated.fits");
	ImagePtr	translatedptr = translatedfile.read();
	Image<RGB<unsigned char> >	*translated
		= dynamic_cast<Image<RGB<unsigned char> > *>(&*translatedptr);
	LuminanceAdapter<RGB<unsigned char>, double>	doubletranslated(*translated);
	DerivativeNormAdapter<double>	i2(doubleimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test chart read");

	// create a phase correlator
	PhaseCorrelator	pc(false);
	std::pair<Point, double>	result = pc(i1, i2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s, weight = %f",
		result.first.toString().c_str(), result.second);

	// expected result: (15.7,47.1)
	debug(LOG_DEBUG, DEBUG_LOG, 0, "sun offset %s (should be (15.7,47.1))",
		result.first.toString().c_str());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end testSun()");
}

void	PhaseCorrelatorTest::testJupiter() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin testJupiter()");

	FITSin	imagefile("testimages/jupiter.fits");
	ImagePtr	imageptr = imagefile.read();
	Image<RGB<unsigned char> >	*image
		= dynamic_cast<Image<RGB<unsigned char> > *>(&*imageptr);
	LuminanceAdapter<RGB<unsigned char>, double>	doubleimage(*image);
	DerivativeNormAdapter<double>	i1(doubleimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test image read");

	FITSin	translatedfile("testimages/jupiter-translated.fits");
	ImagePtr	translatedptr = translatedfile.read();
	Image<RGB<unsigned char> >	*translated
		= dynamic_cast<Image<RGB<unsigned char> > *>(&*translatedptr);
	LuminanceAdapter<RGB<unsigned char>, double>	doubletranslated(*translated);
	DerivativeNormAdapter<double>	i2(doubletranslated);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test chart read");

	// create a phase correlator
	PhaseCorrelator	pc(false);
	std::pair<Point, double>	result = pc(i1, i2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s, weight = %f",
		result.first.toString().c_str(), result.second);

	// expected result: (15.7,47.1)
	debug(LOG_DEBUG, DEBUG_LOG, 0, "Jupiter offset %s (should be (27.1,-15.9))",
		result.first.toString().c_str());

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end testJupiter()");
}

void	PhaseCorrelatorTest::testJupiterSequence() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin testJupiterSequence()");

	FITSin	imagefile("testimages/jupiter-00.fits");
	ImagePtr	imageptr = imagefile.read();
	Image<unsigned char>	*image
		= dynamic_cast<Image<unsigned char> *>(&*imageptr);
	LuminanceAdapter<unsigned char, double>	doubleimage(*image);
	DerivativeNormAdapter<double>	i1(doubleimage);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test image read");

	for (int i = 0; i < 73; i++) {
		std::string	f = stringprintf("testimages/jupiter-%02d.fits",
					i);

		FITSin	translatedfile(f);
		ImagePtr	translatedptr = translatedfile.read();
		Image<unsigned char>	*translated
		= dynamic_cast<Image<unsigned char> *>(&*translatedptr);
		LuminanceAdapter<unsigned char, double>	doubletranslated(*translated);
		DerivativeNormAdapter<double>	i2(doubletranslated);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "test chart read");

		// create a phase correlator
		PhaseCorrelator	pc(false);
		std::pair<Point, double>	result = pc(i1, i2);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s, weight = %f",
			result.first.toString().c_str(), result.second);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end testJupiterSequence()");
}

void	PhaseCorrelatorTest::testDisks() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin testDisks()");
	int	N = 64;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);
	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			double	r = hypot(x - 32, y - 32);
			if (r < 10) {
				fromimage.pixel(x, y) = 100;
			} else if (r > 12) {
				fromimage.pixel(x, y) = 0;
			} else {
				fromimage.pixel(x, y) = 100 * (12 - r) / 2;
			}
			r = hypot(x - 40, y - 48);
			if (r < 10) {
				toimage.pixel(x, y) = 100;
			} else if (r > 12) {
				toimage.pixel(x, y) = 0;
			} else {
				toimage.pixel(x, y) = 100 * (12 - r) / 2;
			}
		}
	}
	DerivativeNormAdapter<double>	from(fromimage);
	DerivativeNormAdapter<double>	to(toimage);

	// create a differential phase correlator
	PhaseCorrelator	pc(false);
	std::pair<Point, double>	result = pc(from, to);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"offset = %s (should be (8,16)), weight = %f",
		result.first.toString().c_str(), result.second);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end testDisks()");
}

void	PhaseCorrelatorTest::testTriangle() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin testTriangle()");
	int	N = 200;
	Image<double>	fromimage(N, N);
	Image<double>	toimage(N, N);

	BarycentricCoordinates	bcfrom(Point(70,70), Point(130,80),
					Point(80,130));
	BarycentricCoordinates	bcto(Point(75,77), Point(135,87),
					Point(85,137));

	for (int x = 0; x < fromimage.size().width(); x++) {
		for (int y = 0; y < fromimage.size().height(); y++) {
			if (bcfrom(Point(x, y)).inside()) {
				fromimage.pixel(x, y) = 1;
			} else {
				fromimage.pixel(x, y) = 0;
			}
			if (bcto(Point(x, y)).inside()) {
				toimage.pixel(x, y) = 1;
			} else {
				toimage.pixel(x, y) = 0;
			}
		}
	}

	DerivedPhaseCorrelator<DerivativeNormAdapter<double> >	pc(true);
	std::pair<Point, double>	result = pc(fromimage, toimage);

	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"offset = %s (should be (8,16)), weight = %f",
		result.first.toString().c_str(), result.second);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end testTriangle()");
}

void	PhaseCorrelatorTest::testMoon() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "beginMoon test");

	FITSin	imagefile("testimages/moon-small.fits");
	ImagePtr	imageptr = imagefile.read();
	Image<unsigned char>	*image = dynamic_cast<Image<unsigned char> *>(&*imageptr);
	TypeReductionAdapter<double, unsigned char>	doubleimage(*image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test image read");

	int	from = 10;
	int	to = 10;

	for (int dx = from; dx <= to; dx++) {
		for (int dy = from; dy <= to; dy++) {
			Point	t(dx + 0.7,dy + 0.1);
			TranslationAdapter<double>	ta(doubleimage, t);
		

			// create a phase correlator
			DerivedPhaseCorrelator<AbsoluteLaplaceAdapter<double> >	pc(true);
			std::pair<Point, double>	result = pc(doubleimage, ta);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s, weight = %f",
				result.first.toString().c_str(), result.second);

			// expected result: (dx, dy)
			Point	delta = result.first - t;
			debug(LOG_DEBUG, DEBUG_LOG, 0, "delta = %s",
				delta.toString().c_str());
		}
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "endMoon test");
}

void	PhaseCorrelatorTest::testOrion() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "beginOrion test");

	FITSin	image1file("testimages/orion1.fits");
	ImagePtr	image1ptr = image1file.read();
	Image<unsigned short>	*image1 = dynamic_cast<Image<unsigned short> *>(&*image1ptr);
	if (NULL == image1) {
		throw std::runtime_error("image1: wrong type");
	}
	TypeReductionAdapter<double, unsigned short>	from(*image1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test image read");

	FITSin	image2file("testimages/orion2.fits");
	ImagePtr	image2ptr = image2file.read();
	Image<unsigned short>	*image2 = dynamic_cast<Image<unsigned short> *>(&*image2ptr);
	if (NULL == image2) {
		throw std::runtime_error("image2: wrong type");
	}
	TypeReductionAdapter<double, unsigned short>	to(*image2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "test chart read");

	// create a phase correlator
	PhaseCorrelator	pc(false);
	std::pair<Point, double>	result = pc(from, to);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "offset = %s, weight = %f",
		result.first.toString().c_str(), result.second);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "endOrion test");
}
} // namespace test
} // namespace astro
