/*
 * FocusableImageConverter.cpp -- test pixel conversions for focus images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <math.h>
#include <AstroDebug.h>
#include <AstroFocus.h>

using namespace astro::focusing;

namespace astro {
namespace test {

class FocusableImageConverter : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testBayer();

	CPPUNIT_TEST_SUITE(FocusableImageConverter);
	CPPUNIT_TEST(testBayer);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FocusableImageConverter);

void	FocusableImageConverter::setUp() {
}

void	FocusableImageConverter::tearDown() {
}

void	FocusableImageConverter::testBayer() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBayer() begin");
	Image<unsigned char>	*img = new Image<unsigned char>(20, 20);
	for (int x = 0; x < 20; x++) {
		for (int y = 0; y < 20; y++) {
			img->pixel(x, y) = 0;
		}
	}
	ImagePtr	imgptr(img);
	img->setMosaicType(MosaicType::BAYER_RGGB);
	int	x = 5, y = 5;
	img->pixel(x,y) = 100;
	double	v = img->pixel(x,y);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "img(%d,%d) = %f", x, y, v);
	focusing::FocusableImageConverterPtr	fc
		= focusing::FocusableImageConverter::get();
	FocusableImage	i = (*fc)(imgptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x, y,
		i->pixel(x,y));
	CPPUNIT_ASSERT(fabs(i->pixel(x,y)) < 0.001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x-1, y,
		i->pixel(x-1,y));
	CPPUNIT_ASSERT(fabs(i->pixel(x-1,y)) < 0.001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x+1, y,
		i->pixel(x+1,y));
	CPPUNIT_ASSERT(fabs(i->pixel(x+1,y)) < 0.001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x, y-1,
		i->pixel(x,y-1));
	CPPUNIT_ASSERT(fabs(i->pixel(x,y-1)) < 0.001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x, y+1,
		i->pixel(x,y+1));
	CPPUNIT_ASSERT(fabs(i->pixel(x,y+1)) < 0.001);

	img->setMosaicType(MosaicType::BAYER_GRBG);
	i = (*fc)(imgptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x, y,
		i->pixel(x,y));
	CPPUNIT_ASSERT(fabs(i->pixel(x,y) - 100) < 0.001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x-1, y,
		i->pixel(x-1,y));
	CPPUNIT_ASSERT(fabs(i->pixel(x-1,y) - 25) < 0.001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x+1, y,
		i->pixel(x+1,y));
	CPPUNIT_ASSERT(fabs(i->pixel(x+1,y) - 25) < 0.001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x, y-1,
		i->pixel(x,y-1));
	CPPUNIT_ASSERT(fabs(i->pixel(x,y-1) - 25) < 0.001);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "i(%d,%d) = %f", x, y+1,
		i->pixel(x,y+1));
	CPPUNIT_ASSERT(fabs(i->pixel(x,y+1) - 25) < 0.001);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testBayer() end");
}

} // namespace test
} // namespace astro
