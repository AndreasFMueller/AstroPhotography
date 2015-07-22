/*
 * ImageRepoTest.cpp -- Tests for the ImageRepo class
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
using namespace astro::camera;

namespace astro {
namespace test {

class ImageRepoTest: public CppUnit::TestFixture {
	std::string	databasename;
	Database	database;
	std::string	directory;
public:
	void	setUp();
	void	tearDown();
	void	testScan();
	void	testImage();
	void	testSelect();
	void	testRemove();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ImageRepoTest);
	CPPUNIT_TEST(testScan);
	CPPUNIT_TEST(testImage);
	CPPUNIT_TEST(testSelect);
	CPPUNIT_TEST(testRemove);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageRepoTest);

void	ImageRepoTest::setUp() {
	databasename = std::string("imagerepo.db");
	char	path[MAXPATHLEN];
	directory = std::string(getcwd(path, MAXPATHLEN));
	database = DatabaseFactory::get(databasename);
}

void	ImageRepoTest::tearDown() {
	database.reset();
}

void	ImageRepoTest::testScan() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testScan() begin");

	debug(LOG_DEBUG, DEBUG_LOG, 0, "scan directory %s",
		directory.c_str());
	
	ImageRepo	repo("repotest", database, directory);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "testScan() end");
}

void	ImageRepoTest::testImage() {
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

	ImageRepo	repo("repotest", database, directory, false);

	long	imageid = repo.save(imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "1st image saved: %d", imageid);
	// add the same image for times
	debug(LOG_DEBUG, DEBUG_LOG, 0, "uuid: %s", ((std::string)(imageptr->getMetadata("UUID"))).c_str());
	imageptr->removeMetadata("UUID");
	repo.save(imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "2nd image saved");
	imageptr->removeMetadata("UUID");
	repo.save(imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "3rd image saved");
	imageptr->removeMetadata("UUID");
	repo.save(imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "4th image saved");
	imageptr->removeMetadata("UUID");
	repo.save(imageptr);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "5th image saved");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "imageid = %ld", imageid);

	ImagePtr	image2 = repo.getImage(imageid);
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

void	ImageRepoTest::testSelect() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSelect() begin");
	ImageRepo	repo("repotest", database, directory, false);
	ImageSpec	spec;
	spec.purpose(Exposure::dark);
	spec.temperature(-47);
	std::set<ImageEnvelope>	resultset = repo.get(spec);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d darks with temperature -47",
		resultset.size());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSelect() end");
}

void	ImageRepoTest::testRemove() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRemove() begin");
	ImageRepo	repo("repotest", database, directory, false);
	ImageSpec	spec;
	spec.purpose(Exposure::dark);
	std::set<ImageEnvelope>	resultset = repo.get(spec);
	std::set<ImageEnvelope>::const_iterator	ii;
	for (ii = resultset.begin(); ii != resultset.end(); ii++) {
		repo.remove(ii->id());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRemove() end");
}

#if 0
void	ImageRepoTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
