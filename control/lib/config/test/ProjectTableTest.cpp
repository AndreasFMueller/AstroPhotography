/*
 * ProjectTableTest.cpp -- Test the project table 
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <AstroDebug.h>
#include <includes.h>
#include "../ProjectTable.h"

using namespace astro::persistence;
using namespace astro::project;

namespace astro {
namespace test {

class ProjectTableTest: public CppUnit::TestFixture {
public:
	void	setUp();
	void	tearDown();
	void	testProject();
	//void	testXXX();

	CPPUNIT_TEST_SUITE(ProjectTableTest);
	CPPUNIT_TEST(testProject);
	//CPPUNIT_TEST(testXXX);
	CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ProjectTableTest);

void	ProjectTableTest::setUp() {
}

void	ProjectTableTest::tearDown() {
}

void	ProjectTableTest::testProject() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testProject() begin");
	unlink("projecttest.db");
	Database	database = DatabaseFactory::get("projecttest.db");
	ProjectTable	projects(database);
	ProjectRecord	project1;
	project1.name = "project1";
	project1.description = "Description of project1";
	project1.started = time(NULL) - 86400;
	long	id1 = projects.add(project1);
	ProjectRecord	project2;
	project2.name = "project2";
	project2.description = "Description of project2";
	project2.started = time(NULL) - 10 * 86400;
	long	id2 = projects.add(project2);
	ProjectRecord	project3 = projects.byid(id1);
	CPPUNIT_ASSERT(project1.name == project3.name);
	CPPUNIT_ASSERT(project1.description == project3.description);
	CPPUNIT_ASSERT(project1.started == project3.started);
	ProjectRecord	project4 = projects.byid(id2);
	CPPUNIT_ASSERT(project2.name == project4.name);
	CPPUNIT_ASSERT(project2.description == project4.description);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d - %d = %d",
		project2.started, project4.started,
		project2.started - project4.started);
	CPPUNIT_ASSERT(project2.started == project4.started);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testProject() end");
}

#if 0
void	ProjectTableTest::testXXX() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() begin");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testXXX() end");
}
#endif

} // namespace test
} // namespace astro
