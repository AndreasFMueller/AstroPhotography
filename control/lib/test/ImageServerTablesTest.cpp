/*
 * ImageServerTablesTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <ImageServerTables.h>
#include <includes.h>

using namespace astro::project;
using namespace astro::persistence;

namespace astro {
namespace test {

class ImageServerTablesTest : public CppUnit::TestFixture {
	Database	database;
public:
	void	setUp();
	void	tearDown();
	void	testImageServerTable();
	void	testMetadataTable();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ImageServerTablesTest);
	CPPUNIT_TEST(testImageServerTable);
	CPPUNIT_TEST(testMetadataTable);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageServerTablesTest);

std::string	dbfilename("imageservertest.db");

void	ImageServerTablesTest::setUp() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up clean database");
	unlink(dbfilename.c_str());
	database = DatabaseFactory::get(dbfilename);
}

void	ImageServerTablesTest::tearDown() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying test database");
	database.reset();
//	unlink(dbfilename.c_str());
}

void	ImageServerTablesTest::testImageServerTable() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImageServerTable() begin");
	ImageTable	images(database);
	ImageRecord	imageinfo1;
	imageinfo1.filename = "testfile.fits";
	imageinfo1.project = "testproject";
	imageinfo1.created = time(NULL);
	imageinfo1.width = 360;
	imageinfo1.height = 240;
	imageinfo1.depth = 1;
	imageinfo1.pixeltype = 8;
	imageinfo1.exposuretime = 47.11;
	imageinfo1.temperature = -47.11;
	imageinfo1.category = "light";
	imageinfo1.bayer = "RGGB";
	imageinfo1.observation = "1962-02-14T12:34:56.777";
	long	id = images.add(imageinfo1);
	for (int count = 0; count < 10; count++) {
		imageinfo1.filename = stringprintf("test%d.fits", count);
		images.add(imageinfo1);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "added object %ld", id);
	ImageRecord imageinfo2 = images.byid(id);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filenames: '%s' ?= '%s'",
		imageinfo1.filename.c_str(),
		imageinfo2.filename.c_str());
	imageinfo1.filename = "testfile.fits";
	CPPUNIT_ASSERT(imageinfo1.filename == imageinfo2.filename);
	CPPUNIT_ASSERT(imageinfo1.project == imageinfo2.project);
	CPPUNIT_ASSERT(imageinfo1.created == imageinfo2.created);
	CPPUNIT_ASSERT(imageinfo1.width == imageinfo2.width);
	CPPUNIT_ASSERT(imageinfo1.height == imageinfo2.height);
	CPPUNIT_ASSERT(imageinfo1.depth == imageinfo2.depth);
	CPPUNIT_ASSERT(imageinfo1.pixeltype == imageinfo2.pixeltype);
	CPPUNIT_ASSERT(imageinfo1.exposuretime == imageinfo2.exposuretime);
	CPPUNIT_ASSERT(imageinfo1.temperature == imageinfo2.temperature);
	CPPUNIT_ASSERT(imageinfo1.category == imageinfo2.category);
	CPPUNIT_ASSERT(imageinfo1.bayer == imageinfo2.bayer);
	CPPUNIT_ASSERT(imageinfo1.observation == imageinfo2.observation);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImageServerTable() end");
}

void	ImageServerTablesTest::testMetadataTable() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMetadataTable() begin");
	ImageTable	images(database);
	ImageRecord	imageinfo1;
	imageinfo1.filename = "metatest.fits";
	imageinfo1.project = "testproject";
	imageinfo1.created = time(NULL);
	imageinfo1.width = 360;
	imageinfo1.height = 240;
	imageinfo1.depth = 1;
	imageinfo1.pixeltype = 8;
	imageinfo1.exposuretime = 47.11;
	imageinfo1.temperature = -47.11;
	imageinfo1.category = "light";
	imageinfo1.bayer = "RGGB";
	imageinfo1.observation = "1962-02-14T12:34:56.777";
	long	id = images.add(imageinfo1);
	MetadataTable	metadata(database);
	MetadataRecord	meta(-1, id);

	meta.seqno = 0;
	meta.key = "EXPTIME";
	meta.value = "47.11";
	meta.comment = "exposure time in seconds";
	metadata.add(meta);

	meta.seqno = 1;
	meta.key = "BAYER";
	meta.value = "'RGGB'";
	meta.comment = "Bayer matrix layout";
	metadata.add(meta);

	meta.seqno = 2;
	meta.key = "SET-TEMP";
	meta.value = "-50.000";
	meta.comment = "set temperature";
	metadata.add(meta);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMetadataTable() end");
}

#if 0
void	ImageServerTablesTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
