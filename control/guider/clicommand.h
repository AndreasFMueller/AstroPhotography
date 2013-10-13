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

namespace astro {
namespace cli {

class clicommand {
	std::string	name;
public:
	clicommand(const std::string& _name) : name(_name) { }
	~clicommand() { }
	virtual void	operator()(const std::string& command,
				const std::vector<std::string>& args) { }
};

class command_error : public std::runtime_error {
public:
	command_error(const std::string& cause) : std::runtime_error(cause) { }
};

} // namespace cli
} // namespace astro


#endif /* _clicommand_h */
