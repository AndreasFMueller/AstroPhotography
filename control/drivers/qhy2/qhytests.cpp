/*
 * qhytests.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <includes.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <stdexcept>

using namespace astro;

double	default_exposure = 0.01;
int	default_imagecount = 5;
const char	*default_targetdirectory = "../../tmp";
const char	*default_prefix = "test";

/**
 * \brief Make sure target directory exists
 *
 * checks whether the directory specified as argument exists, and if not
 * creates it. 
 */
void	check_directory(const char *directory) {
	struct stat	sb;
	if (0 == stat(default_targetdirectory, &sb)) {
		if (S_ISDIR(sb.st_mode)) {
			return;
		}
		std::string	msg = stringprintf("%s: not a directory",
			directory);
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg.c_str());
	}
	if (0 == mkdir(directory, 0777)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s created", directory);
		return;
	}
	std::string	msg = stringprintf("could not create %s: %s", directory,
		strerror(errno));
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

int	main(int argc, char *argv[]) {
	int	c;
	while (EOF != (c = getopt(argc, argv, "de:n:t:p:")))
		switch (c) {
		case 'd':
			debuglevel = LOG_DEBUG;
			break;
		case 'e':
			default_exposure = atof(optarg);
			break;
		case 'n':
			default_imagecount = atoi(optarg);
			break;
		case 't':
			default_targetdirectory = optarg;
			break;
		case 'p':
			default_prefix = optarg;
			break;
		}

	// if the target directory is set, we have to make sure that it 
	// really exists, and is a directory
	if (default_targetdirectory) {
		check_directory(default_targetdirectory);
	}

	debug(LOG_DEBUG, DEBUG_LOG, 0, "qhy2 tests");
	CppUnit::TextUi::TestRunner	runner;
	CppUnit::TestFactoryRegistry	&registry
		= CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	bool	wasSuccessful = runner.run("", false);
	return (wasSuccessful) ? 0 : 1;
}

