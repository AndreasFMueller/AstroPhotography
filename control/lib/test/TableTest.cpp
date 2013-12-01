/*
 * TableTest.cpp -- tests for the Table class
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUtils.h>
#include <AstroDebug.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include <AstroPersistence.h>
#include <math.h>
#include <Testtable.h>

using namespace astro::persistence;

namespace astro {
namespace test {

class TableTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testRetrieve();
	void	testInsert();
	void	testUpdate();
	void	testDelete();

	CPPUNIT_TEST_SUITE(TableTest);
	CPPUNIT_TEST(testRetrieve);
	CPPUNIT_TEST(testInsert);
	CPPUNIT_TEST(testUpdate);
	CPPUNIT_TEST(testDelete);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TableTest);

void	TableTest::setUp() {
}

void	TableTest::tearDown() {
}

void	TableTest::testRetrieve() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRetrieve() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	Table<TestEntry, TesttableAdapter>	table(database);
	TestEntry	entry = table.byid(2);
	std::ostringstream	out;
	out << entry;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "object 2: %s", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testRetrieve() end");
}

void	TableTest::testInsert() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInsert() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	Table<TestEntry, TesttableAdapter>	table(database);
	TestEntry	entry(0);
	entry.intfield(1291);
	entry.doublefield(12.91);
	entry.stringfield("Eidgenossenschaft");
	int	objectid = table.add(entry);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new objectid = %d", objectid);
	entry = table.byid(objectid);
	std::ostringstream	out;
	out << entry;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new entry: %s", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInsert() end");
}

void	TableTest::testUpdate() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testUpdate() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	Table<TestEntry, TesttableAdapter>	table(database);
	TestEntry	entry(0);
	entry.intfield(1918);
	entry.doublefield(19.18);
	entry.stringfield("Generalstreik");
	int	objectid = table.add(entry);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new objectid = %d", objectid);
	entry.stringfield("Genf");
	table.update(objectid, entry);
	entry = table.byid(objectid);
	std::ostringstream	out;
	out << entry;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new entry: %s", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testUpdate() end");
}

void	TableTest::testDelete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDelete() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	Table<TestEntry, TesttableAdapter>	table(database);
	long	objectid = table.nextid() - 1;
	table.remove(objectid);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDelete() end");
}

} // namespace test
} // namespace astro
