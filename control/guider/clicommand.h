/*
 * clicommand.h -- common base class for command classes
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _clicommand_h
#define _clicommand_h

#include <vector>
#include <string>
#include <stdexcept>
#include <memory>

namespace astro {
namespace cli {

/**
 * \brief base class for all commands
 */
class clicommand {
	std::string	name;
public:
	clicommand(const std::string& _name) : name(_name) { }
	~clicommand() { }
	virtual void	operator()(const std::string& command,
				const std::vector<std::string>& args) { }
};

/**
 * \brief exception thrown by commands that fail
 */
class command_error : public std::runtime_error {
public:
	command_error(const std::string& cause) : std::runtime_error(cause) { }
};

typedef std::shared_ptr<clicommand>	clicommandptr;

/**
 * \brief command factory
 */
class commandfactory {
public:
	clicommandptr	get(const std::string& commandname,
		const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro


#endif /* _clicommand_h */
