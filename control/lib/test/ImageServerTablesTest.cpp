/*
 * ImageServerTables.cpp -- template for tests
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

class ImageServerTables: public CppUnit::TestFixture {
	Database	database;
public:
	void	setUp();
	void	tearDown();
	void	testImageServerTable();
	void	testMetadataTable();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ImageServerTables);
	CPPUNIT_TEST(testImageServerTable);
	CPPUNIT_TEST(testMetadataTable);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageServerTables);

std::string	dbfilename("imageservertest.db");

void	ImageServerTables::setUp() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "setting up clean database");
	unlink(dbfilename.c_str());
	database = DatabaseFactory::get(dbfilename);
}

void	ImageServerTables::tearDown() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "destroying test database");
	database.reset();
//	unlink(dbfilename.c_str());
}

void	ImageServerTables::testImageServerTable() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImageServerTable() begin");
	ImageServerTable	images(database);
	ImageServerRecord	imageserverinfo1;
	imageserverinfo1.filename = "testfile.fits";
	imageserverinfo1.project = "testproject";
	imageserverinfo1.created = time(NULL);
	imageserverinfo1.width = 360;
	imageserverinfo1.height = 240;
	imageserverinfo1.depth = 1;
	imageserverinfo1.pixeltype = 8;
	imageserverinfo1.exposuretime = 47.11;
	imageserverinfo1.temperature = -47.11;
	imageserverinfo1.category = "light";
	imageserverinfo1.bayer = "RGGB";
	imageserverinfo1.observation = "1962-02-14T12:34:56.777";
	long	id = images.add(imageserverinfo1);
	for (int count = 0; count < 10; count++) {
		imageserverinfo1.filename = stringprintf("test%d.fits", count);
		images.add(imageserverinfo1);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "added object %ld", id);
	ImageServerRecord imageserverinfo2 = images.byid(id);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "filenames: '%s' ?= '%s'",
		imageserverinfo1.filename.c_str(),
		imageserverinfo2.filename.c_str());
	imageserverinfo1.filename = "testfile.fits";
	CPPUNIT_ASSERT(imageserverinfo1.filename == imageserverinfo2.filename);
	CPPUNIT_ASSERT(imageserverinfo1.project == imageserverinfo2.project);
	CPPUNIT_ASSERT(imageserverinfo1.created == imageserverinfo2.created);
	CPPUNIT_ASSERT(imageserverinfo1.width == imageserverinfo2.width);
	CPPUNIT_ASSERT(imageserverinfo1.height == imageserverinfo2.height);
	CPPUNIT_ASSERT(imageserverinfo1.depth == imageserverinfo2.depth);
	CPPUNIT_ASSERT(imageserverinfo1.pixeltype == imageserverinfo2.pixeltype);
	CPPUNIT_ASSERT(imageserverinfo1.exposuretime == imageserverinfo2.exposuretime);
	CPPUNIT_ASSERT(imageserverinfo1.temperature == imageserverinfo2.temperature);
	CPPUNIT_ASSERT(imageserverinfo1.category == imageserverinfo2.category);
	CPPUNIT_ASSERT(imageserverinfo1.bayer == imageserverinfo2.bayer);
	CPPUNIT_ASSERT(imageserverinfo1.observation == imageserverinfo2.observation);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testImageServerTable() end");
}

void	ImageServerTables::testMetadataTable() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testMetadataTable() begin");
	ImageServerTable	images(database);
	ImageServerRecord	imageserverinfo1;
	imageserverinfo1.filename = "metatest.fits";
	imageserverinfo1.project = "testproject";
	imageserverinfo1.created = time(NULL);
	imageserverinfo1.width = 360;
	imageserverinfo1.height = 240;
	imageserverinfo1.depth = 1;
	imageserverinfo1.pixeltype = 8;
	imageserverinfo1.exposuretime = 47.11;
	imageserverinfo1.temperature = -47.11;
	imageserverinfo1.category = "light";
	imageserverinfo1.bayer = "RGGB";
	imageserverinfo1.observation = "1962-02-14T12:34:56.777";
	long	id = images.add(imageserverinfo1);
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
void	ImageServerTables::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
