/*
 * ImageServerTableTest.cpp -- template for tests
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <includes.h>
#include <AstroPersistence.h>
#include <ImageServersTable.h>

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace test {

class ImageServerTableTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testTable();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ImageServerTableTest);
	CPPUNIT_TEST(testTable);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ImageServerTableTest);

void	ImageServerTableTest::setUp() {
}

void	ImageServerTableTest::tearDown() {
}

void	ImageServerTableTest::testTable() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTable() begin");
	unlink("imageservers.db");
	Database	database = DatabaseFactory::get("imageservers.db");
	ImageServerTable	servers(database);
	ImageServerRecord	server1;
	server1.servername = "test1";
	server1.database = "test1.db";
	server1.directory = "test1dir";
	long	id1 = servers.add(server1);
	CPPUNIT_ASSERT(id1 == 1);
	ImageServerRecord	server2;
	server2.servername = "test2";
	server2.database = "test2.db";
	server2.directory = "test2dir";
	long	id2 = servers.add(server2);
	CPPUNIT_ASSERT(id2 == 2);
	ImageServerRecord	server3 = servers.byid(id1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d", server1.id());
	CPPUNIT_ASSERT(server1.servername == server3.servername);
	CPPUNIT_ASSERT(server1.database == server3.database);
	CPPUNIT_ASSERT(server1.directory == server3.directory);
	CPPUNIT_ASSERT(server3.id() == id1);
	ImageServerRecord	server4 = servers.byid(id2);
	CPPUNIT_ASSERT(server2.servername == server4.servername);
	CPPUNIT_ASSERT(server2.database == server4.database);
	CPPUNIT_ASSERT(server2.directory == server4.directory);
	CPPUNIT_ASSERT(server4.id() == id2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testTable() end");
}

#if 0
void	ImageServerTableTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
