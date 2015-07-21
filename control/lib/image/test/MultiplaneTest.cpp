/*
 * MultiplaneTest.cpp -- read FITS file tests
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */

#include <AstroIO.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <unistd.h>
#include <iostream>
#include <AstroDebug.h>

using namespace astro::io;
using namespace astro::image;

namespace astro {
namespace test {

class MultiplaneTest : public CppUnit::TestFixture {
public:
	void	setUp() { }
	void	tearDown() { }
	void	testWrite();
	void	testRead();

	CPPUNIT_TEST_SUITE(MultiplaneTest);
	CPPUNIT_TEST(testWrite);
	CPPUNIT_TEST(testRead);
	CPPUNIT_TEST_SUITE_END();
};

void	MultiplaneTest::testWrite() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWrite() begin");
	Image<Multiplane<unsigned char, 2> >	*imagep
		= new Image<Multiplane<unsigned char, 2> >(20, 30);
	ImagePtr	image(imagep);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new %ux%u image",
		image->size().width(), image->size().height());
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			Multiplane<unsigned char, 2>	v;
			v.p[0] = 2 * x * y;
			v.p[1] = 2 * x * y + 1;
			imagep->pixel(x, y) = v;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image filled with values");
	const char	*filename = "tmp/multiplane.fits";
	unlink(filename);
	FITSout	out(filename);
	out.write(image);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testWrite() end");
}

void	MultiplaneTest::testRead() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRead() begin");
	FITSin	in("tmp/multiplane.fits");
	ImagePtr	image = in.read();
	Image<Multiplane<unsigned char, 2> >	*imagep
		= dynamic_cast<Image<Multiplane<unsigned char, 2> > *>(&*image);
	CPPUNIT_ASSERT(imagep != NULL);
	for (unsigned int x = 0; x < image->size().width(); x++) {
		for (unsigned int y = 0; y < image->size().height(); y++) {
			Multiplane<unsigned char, 2>	v;
			v.p[0] = 2 * x * y;
			v.p[1] = 2 * x * y + 1;
			CPPUNIT_ASSERT(imagep->pixel(x, y) == v);
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRead() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(MultiplaneTest);

} // namespace io
} // namespace astro
