/*
 * MedianRadiusAdapterTest.cpp -- test image transforms
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroAdapter.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <AstroConvolve.h>
#include <iostream>

using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace test {

class MedianRadiusAdapterTest : public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();

	void	testFilter();
	void	testMask();
	void	testMapping();

	CPPUNIT_TEST_SUITE(MedianRadiusAdapterTest);
	//CPPUNIT_TEST(testFilter);
	CPPUNIT_TEST(testMask);
	CPPUNIT_TEST(testMapping);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MedianRadiusAdapterTest);

void	MedianRadiusAdapterTest::setUp() {
}

void	MedianRadiusAdapterTest::tearDown() {
}

void	MedianRadiusAdapterTest::testFilter() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin MedianRadiusAdapter filter test");
	FITSin	imagefile("m42-luminance.fits");
	ImagePtr	imageptr = imagefile.read();
	Image<float>	*image = dynamic_cast<Image<float> *>(&*imageptr);
	if (NULL == image) {
		throw std::runtime_error("bad image type");
	}
	adapter::MedianRadiusAdapter<float>	mra(*image, 10);
	Image<float>	*destarred = new Image<float>(mra);
	ImagePtr	destarredptr(destarred);
	FITSout	outfile("tmp/m42-destarred.fits");
	outfile.setPrecious(false);
	outfile.write(destarredptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end MedianRadiusAdapter filter test");
}

void	MedianRadiusAdapterTest::testMask() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin MedianRadiusAdapter filter test");
	FITSin	infile("tmp/m42-destarred.fits");
	ImagePtr	imageptr = infile.read();
	Image<float>	*image = dynamic_cast<Image<float>*>(&*imageptr);
	if (NULL == image) {
		throw std::runtime_error("bad image type");
	}

	// convert using Fourier
	adapter::TypeConversionAdapter<float>	im(*image);
	FourierImage	fimage(im);

	// gaussian blurr
	TiledGaussImage	g(imageptr->size(), 4, 1);
	//GaussImage	g(imageptr->size(), ImagePoint(), 10, 1);
        Image<double>   *b = new Image<double>(g);
	FITSout		gout("tmp/m42-gauss.fits");
	ImagePtr	bptr(b);
	gout.setPrecious(false);
	gout.write(bptr);
	FourierImage	blurr(*b);

	FourierImagePtr	blurred = fimage * blurr;
	ImagePtr	outimageptr = blurred->inverse();

	FITSout	outfile("tmp/m42-smoothed.fits");
	outfile.setPrecious(false);
	outfile.write(outimageptr);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "end MedianRadiusAdapter filter test");
}

template<typename T, typename S>
class SmoothingAdapter : public ConstImageAdapter<T> {
	const ConstImageAdapter<T>&	_image;
	const ConstImageAdapter<S>&	_smoother;
public:
	SmoothingAdapter(const ConstImageAdapter<T>& image,
		const ConstImageAdapter<S>& smoother)
		: ConstImageAdapter<T>(image.getSize()), _image(image),
		  _smoother(smoother) {
	}
	virtual T	pixel(int x, int y) const {
		return _image.pixel(x, y) * (1 / (0.006 * _smoother.pixel(x, y) + 1.));
	}
};

void	MedianRadiusAdapterTest::testMapping() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "begin MedianRadiusAdapter mapping test");
	FITSin	infile("m42-color.fits");
	ImagePtr	imageptr = infile.read();
	Image<RGB<float> >	*image
		= dynamic_cast<Image<RGB<float> >*>(&*imageptr);

	FITSin	smoothfile("tmp/m42-smoothed.fits");
	ImagePtr	smoothptr = smoothfile.read();
	Image<double>	*smooth = dynamic_cast<Image<double>*>(&*smoothptr);
	if (NULL == smooth) {
		throw std::runtime_error("wrong image type");
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "constructing smoothing adapter");
	SmoothingAdapter<RGB<float>, double>	s(*image, *smooth);
	Image<RGB<float> >	*mapped = new Image<RGB<float> >(s);
	ImagePtr	mappedptr(mapped);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image mapped");

	FITSout	outfile("tmp/m42-mapped.fits");
	outfile.setPrecious(false);
	outfile.write(mappedptr);
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "end MedianRadiusAdapter mapping test");
}

} // namespace test
} // namespace astro
