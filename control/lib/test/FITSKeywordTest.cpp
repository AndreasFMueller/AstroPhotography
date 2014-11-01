/*
 * FITSKeywordTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroImage.h>
#include <AstroIO.h>
#include <includes.h>

using namespace astro::io;
using namespace astro::image;

namespace astro {
namespace test {

class FITSKeywordTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testHistory();
	void	testComment();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(FITSKeywordTest);
	CPPUNIT_TEST(testHistory);
	CPPUNIT_TEST(testComment);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FITSKeywordTest);

void	FITSKeywordTest::setUp() {
}

void	FITSKeywordTest::tearDown() {
}

void	FITSKeywordTest::testHistory() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testHistory() begin");

	// create an image
	ImageSize	size(120,80);
	Image<unsigned char>	image(size);
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			image.writablepixel(x, y) = (x + y) % 256;
		}
	}
	Metavalue	value1 = FITSKeywords::meta("HISTORY", "history step");
	image.setMetadata(value1);

	// write the image to a file
	FITSoutfile<unsigned char>	out("history.fits");
	out.setPrecious(false);
	out.write(image);

	// read the image
	FITSinfile<unsigned char>	in("history.fits");
	Image<unsigned char>	*image2 = in.read();
	Metavalue	value3 = image2->getMetadata("HISTORY");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s(%s): %s|%s",
		value3.getKeyword().c_str(), value3.getType().name(),
		value3.getValue().c_str(), value3.getComment().c_str());

	CPPUNIT_ASSERT(value1.getComment() == value3.getComment());

	// cleanup
	delete image2;
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testHistory() end");
}

void	FITSKeywordTest::testComment() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testComment() begin");

	// create an image
	ImageSize	size(120,80);
	Image<unsigned char>	image(size);
	for (unsigned int x = 0; x < size.width(); x++) {
		for (unsigned int y = 0; y < size.height(); y++) {
			image.writablepixel(x, y) = (x + y) % 256;
		}
	}
	image.removeMetadata("COMMENT");
	image.dump_metadata();
	Metavalue	value2 = FITSKeywords::meta("COMMENT",
					"another comment");
	image.setMetadata(value2);

	// write the image to a file
	FITSoutfile<unsigned char>	out("comment.fits");
	out.setPrecious(false);
	out.write(image);

	// read the image
	FITSinfile<unsigned char>	in("comment.fits");
	Image<unsigned char>	*image2 = in.read();
	image2->dump_metadata();

	ImageMetadata::const_iterator	m;
	int	i = 0;
	for (m = image2->begin(); i < 3; m++, i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s(%s): %s|%s",
			m->second.getKeyword().c_str(),
			m->second.getType().name(),
			m->second.getValue().c_str(),
			m->second.getComment().c_str());
#if 0
		if (i == 2) {
			CPPUNIT_ASSERT(m->second.getComment()
				== "another comment");
		}
#endif
	}

	// cleanup
	delete image2;
	
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testComment() end");
}

#if 0
void	FITSKeywordTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
