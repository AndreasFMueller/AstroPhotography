/*
 * RepositoryTest.cpp -- tests for the Repository class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#include <AstroLoader.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <iostream>
#include <includes.h>

using namespace astro::module;

namespace astro {
namespace test {

class RepositoryTest : public CppUnit::TestFixture {
	std::string	path;
public:
	void	setUp();
	void	tearDown();
	void	testPathexists();
	void	testPathdoesnotexist();
	void	testModules();

	CPPUNIT_TEST_SUITE(RepositoryTest);
	CPPUNIT_TEST(testPathexists);
	CPPUNIT_TEST_EXCEPTION(testPathdoesnotexist, repository_error);
	CPPUNIT_TEST(testModules);
	CPPUNIT_TEST_SUITE_END();
};

static const char	*files[11] = {
	"libastro.a",
	"libmock1.a",
	"libmock1.la",
	"libmock1.so",
	"libmock1.so.0",
	"libmock1.so.0.0.0",
	"libmock2.a",
	"libmock2.la",
	"libmock2.so",
	"libmock2.so.0",
	"libmock2.so.0.0.0"
};

void	RepositoryTest::setUp() {
	// create a temporary directory
	char	dirname[32];
	strcpy(dirname, "/tmp/astroXXXXXX");
	char	*modifieddirname = mkdtemp(dirname);
	if (NULL == modifieddirname) {
		fprintf(stderr, "cannot create a work directory\n");
		return;
	}
	path = std::string(dirname);

	// construct a repository on it
	Repository	repository(path);

	// add some files to it
	for (int i = 0; i < 11; i++) {
		char	filename[1024];
		snprintf(filename, sizeof(filename), "%s/%s",
			dirname, files[i]);
		int	fd = creat(filename, 0666);
		CPPUNIT_ASSERT(fd >= 0);

		// if the filename ends in .la, we write something to it
		if (i == 2) {
			char	*contents =	"# test file\n"
						"dlname='libmock1.so.0\n";
			write(fd, contents, strlen(contents));
		}

		if (i == 7) {
			char	*contents =	"# test file\n"
						"# with illegal dlname\n"
						"dlname='libmock2.so.0'\n";
			write(fd, contents, strlen(contents));
		}

		close(fd);
	}
}

void	RepositoryTest::tearDown() {
	// remove everything below the path
	DIR	*dir = opendir(path.c_str());
	struct dirent	*dirent;
	while (dirent = readdir(dir)) {
		char	filename[1024];
		snprintf(filename, sizeof(filename), "%s/%s", path.c_str(),
			dirent->d_name);
		unlink(filename);
	}
	if (rmdir(path.c_str()) < 0) {
		fprintf(stderr, "cannot remove directory %s: %s\n",
			path.c_str(), strerror(errno));;
	}
}

void	RepositoryTest::testPathexists() {
	Repository	repository(".");
	repository.modules();
}

void	RepositoryTest::testPathdoesnotexist() {
	Repository	repository("./this/path/quite/certainly/does/not/exit");
	repository.modules();
}


void	RepositoryTest::testModules() {
	// query the list of modules, and verify it's contents
	Repository	repository(path);
	std::vector<ModulePtr>	m = repository.modules();
	CPPUNIT_ASSERT(m.size() == 1);

	CPPUNIT_ASSERT(!repository.contains("libmock1"));
	CPPUNIT_ASSERT(repository.contains("libmock2"));
	CPPUNIT_ASSERT(!repository.contains("blubb"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(RepositoryTest);

} // namespace test
} // namespace astro
