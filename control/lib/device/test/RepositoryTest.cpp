/*
 * ModuleRepositoryTest.cpp -- tests for the ModuleRepository class
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroLoader.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <config.h>
#include <iostream>
#include <includes.h>
#include <AstroDebug.h>

using namespace astro::module;

namespace astro {
namespace test {

class ModuleRepositoryTest : public CppUnit::TestFixture {
	std::string	path;
public:
	void	setUp();
	void	tearDown();
	void	testPathexists();
	void	testPathdoesnotexist();
	void	testModules();

	CPPUNIT_TEST_SUITE(ModuleRepositoryTest);
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

void	ModuleRepositoryTest::setUp() {
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
	ModuleRepositoryPtr	repository = getModuleRepository(path);

	// add some files to it
	for (int i = 0; i < 11; i++) {
		char	filename[1024];
		snprintf(filename, sizeof(filename), "%s/%s",
			dirname, files[i]);
		int	fd = creat(filename, 0666);
		CPPUNIT_ASSERT(fd >= 0);

		// if the filename ends in .la, we write something to it
		if (i == 2) {
			const char	*contents =	"# test file\n"
						"dlname='libmock1.so.0\n";
			write(fd, contents, strlen(contents));
		}

		if (i == 7) {
			const char	*contents =	"# test file\n"
						"# with illegal dlname\n"
						"dlname='libmock2.so.0'\n";
			write(fd, contents, strlen(contents));
		}

		close(fd);
	}
}

void	ModuleRepositoryTest::tearDown() {
	// remove everything below the path
	DIR	*dir = opendir(path.c_str());
	if (NULL == dir) {
		std::string	msg = stringprintf("cannot open %s: %s",
			path.c_str(), strerror(errno));
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	struct dirent	*direntp = NULL;
	struct dirent	direntry;
	do {
		int	rc = readdir_r(dir, &direntry, &direntp);
		if (rc) {
			std::string	msg = stringprintf("cannot read dir "
				"%s: %s", path.c_str(), strerror(errno));
			debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
			closedir(dir);
			throw std::runtime_error(msg);
		}
		if (NULL == direntp)
			continue;
		char	filename[1024];
		snprintf(filename, sizeof(filename), "%s/%s", path.c_str(),
			direntp->d_name);
		unlink(filename);
	} while (NULL != direntp);
	if (rmdir(path.c_str()) < 0) {
		fprintf(stderr, "cannot remove directory %s: %s\n",
			path.c_str(), strerror(errno));;
	}
}

void	ModuleRepositoryTest::testPathexists() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPathexists() begin");
	ModuleRepositoryPtr	repository = getModuleRepository(".");
	repository->modules();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPathexists() end");
}

void	ModuleRepositoryTest::testPathdoesnotexist() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPathdoesnotexist() begin");
	ModuleRepositoryPtr	repository = getModuleRepository("./this/path/quite/certainly/does/not/exit");
	repository->modules();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testPathdoesnotexist() end");
}


void	ModuleRepositoryTest::testModules() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testModules() begin");
	// query the list of modules, and verify it's contents
	ModuleRepositoryPtr	repository = getModuleRepository(path);
	std::vector<ModulePtr>	m = repository->modules();
	CPPUNIT_ASSERT(m.size() == 1);

	CPPUNIT_ASSERT(!repository->contains("libmock1"));
	CPPUNIT_ASSERT(repository->contains("libmock2"));
	CPPUNIT_ASSERT(!repository->contains("blubb"));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "testModules() end");
}

CPPUNIT_TEST_SUITE_REGISTRATION(ModuleRepositoryTest);

} // namespace test
} // namespace astro
