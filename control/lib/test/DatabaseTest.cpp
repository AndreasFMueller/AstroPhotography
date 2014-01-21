/*
 * DatabaseTest.cpp -- test the Database class
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

using namespace astro::persistence;

namespace astro {
namespace test {

class DatabaseTest : public CppUnit::TestFixture {
private:
public:
	void	setUp();
	void	tearDown();
	void	testConstructor();
	void	testQuery();
	void	testSelectStatement();
	void	testInsert();
	void	testDelete();

	CPPUNIT_TEST_SUITE(DatabaseTest);
	CPPUNIT_TEST(testConstructor);
	CPPUNIT_TEST(testQuery);
	CPPUNIT_TEST(testSelectStatement);
	CPPUNIT_TEST(testInsert);
	CPPUNIT_TEST(testDelete);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(DatabaseTest);

void	DatabaseTest::setUp() {
}

void	DatabaseTest::tearDown() {
}

void	DatabaseTest::testConstructor() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	std::vector<std::string>	fn = database->fieldnames("testtable");
	std::vector<std::string>::const_iterator	i;
	for (i = fn.begin(); i != fn.end(); i++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "field name: %s", i->c_str());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testConstructor() end");
}

void	DatabaseTest::testQuery() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testQuery() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	Result	result = database->query("select count(*) as 'rowcount' from testtable");
	std::ostringstream	out;
	out << result;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result: %s", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testQuery() end");
}

void	DatabaseTest::testSelectStatement() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSelectStatement() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	StatementPtr	statement
		= database->statement("select * from testtable");
	Result	result = statement->result();
	std::ostringstream	out;
	out << result;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result: %s", out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testSelectStatement() end");
}

void	DatabaseTest::testInsert() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInsert() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	StatementPtr	statement
		= database->statement("insert into testtable(id, intfield, floatfield, stringfield, timefield) values (?, ?, ?, ?, ?)");
	statement->bind(0, (int)44444);
	statement->bind(1, (int)47);
	statement->bind(2, (double)sqrt(47));
	statement->bind(3, std::string("siebenundvierzig"));
	statement->bind(4, std::string("2014-01-01 12:34:56"));
	statement->execute();

	// display 
	statement = database->statement("select * from testtable");
	Result	result2 = statement->result();
	std::ostringstream	out;
	out << result2;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result after insert: %s",
		out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testInsert() end");
}

void	DatabaseTest::testDelete() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDelete() begin");
	DatabaseFactory	dbf;
	Database	database = dbf.get("testdb.db");
	StatementPtr	statement
		= database->statement("delete from testtable where id = ?");
	statement->bind(0, (int)44444);
	statement->execute();
	// display 
	statement = database->statement("select * from testtable");
	Result	result2 = statement->result();
	std::ostringstream	out;
	out << result2;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "result after delete: %s",
		out.str().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testDelete() end");
}

} // namespace test
} // namespace astro
