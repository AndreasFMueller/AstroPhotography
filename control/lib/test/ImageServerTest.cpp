/*
 * ImageServerTest.cpp -- Tests for the ImageServer class
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroProject.h>
#include <AstroIO.h>
#include <includes.h>

using namespace astro::project;
using namespace astro::persistence;
using namespace astro::image;
using namespace astro::io;

namespace astro {
namespace test {

class ImageServerTest: public CppUnit::TestFixture {
	std::string	databasename;
	Database	database;
	std::string	directory;
public:
	void	setUp();
	void	tearDown();
	void	testScan();
	void	testImage();
	void	testSelect();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ImageServerTest);
	CPPUNIT_TEST(testScan);
	CPPUNIT_TEST(testImage);
	CPPUNIT_TEST(testSelect);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageServerTest);

void	ImageServerTest::setUp() {
	databasename = std::string("imageserver.db");
	char	path[MAXPATHLEN];
	directory = std::string(getcwd(path, MAXPATHLEN));
	database = DatabaseFactory::get(databasename);
}

void	ImageServerTest::tearDown() {
	database.reset();
}

void	ImageServerTest::testScan() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testScan() begin");

	debug(LOG_DEBUG, DEBUG_LOG, 0, "scan directory %s",
		directory.c_str());
	
	ImageServer	server(database, directory);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testScan() end");
}

void	ImageServerTest::testImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImage() begin");

	ImageSize	size(360, 240);
	Image<RGB<float> >	*image = new Image<RGB<float> >(size);
	ImagePtr	imageptr(image);
	imageptr->setMetadata(FITSKeywords::meta("PURPOSE", "dark"));
	imageptr->setMetadata(FITSKeywords::meta("PROJECT", "testproject"));
	imageptr->setMetadata(FITSKeywords::meta("EXPTIME", 300.));
	imageptr->setMetadata(FITSKeywords::meta("DATE-OBS",
		"2014-01-02T03:04:05.678"));
	imageptr->setMetadata(FITSKeywords::meta("INSTRUME", "SX"));
	imageptr->setMetadata(FITSKeywords::meta("CCD-TEMP", -47.1));
	imageptr->setMetadata(FITSKeywords::meta("BAYER", "RGGB"));

	ImageServer	server(database, directory, false);

	long	imageid = server.save(imageptr);
	server.save(imageptr);
	server.save(imageptr);
	server.save(imageptr);
	server.save(imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imageid = %ld", imageid);

	ImagePtr	image2 = server.getImage(imageid);
	CPPUNIT_ASSERT(image->getMetadata("PURPOSE")
		== image2->getMetadata("PURPOSE"));
	CPPUNIT_ASSERT(image->getMetadata("PROJECT")
		== image2->getMetadata("PROJECT"));
	CPPUNIT_ASSERT((double)image->getMetadata("EXPTIME")
		== (double)image2->getMetadata("EXPTIME"));
	CPPUNIT_ASSERT(image->getMetadata("DATE-OBS")
		== image2->getMetadata("DATE-OBS"));
	CPPUNIT_ASSERT(image->getMetadata("INSTRUME")
		== image2->getMetadata("INSTRUME"));
	CPPUNIT_ASSERT(image->getMetadata("BAYER")
		== image2->getMetadata("BAYER"));
	CPPUNIT_ASSERT((double)image->getMetadata("CCD-TEMP")
		== (double)image2->getMetadata("CCD-TEMP"));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImage() end");
}

void	ImageServerTest::testSelect() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSelect() begin");
	ImageServer	server(database, directory, false);
	ImageSpec	spec;
	spec.category(ImageSpec::dark);
	spec.temperature(-47);
	std::set<ImageEnvelope>	resultset = server.get(spec);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d darks with temperature -47",
		resultset.size());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSelect() end");
}

#if 0
void	ImageServerTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
