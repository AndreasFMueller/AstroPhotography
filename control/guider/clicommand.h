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
#include <map>

namespace astro {
namespace cli {

class commandfactory;

/**
 * \brief base class for all commands
 *
 * The base class collects common information used by all classes, and 
 * it defines the interface use e.g. by the factory or the help system
 */
class clicommand {
	commandfactory&	_factory;
	std::string	name;
public:
	clicommand(commandfactory& factory,
		const std::string& _name) : _factory(factory), name(_name) { }
	~clicommand() { }
	commandfactory&	factory() { return _factory; }
	virtual void	operator()(const std::string& command,
				const std::vector<std::string>& args) = 0;
	virtual std::string	summary() const = 0;
	virtual std::string	help() const = 0;
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
 * \brief key for command map
 *
 * We derive this from the pair template, which already does what we need.
 * We just need to override the comparison operators to get lexical ordering
 * in ordered containers. We also add accessors for the pair members so
 * that we can use more suggestive names for them.
 */
class commandkey : std::pair<std::string, std::string> {
public:
	commandkey(const std::string& commandname);
	commandkey(const std::string& commandname,
		const std::string& subcommandname);
	bool	operator==(const commandkey& other) const;
	bool	operator<(const commandkey& other) const;
	const std::string&	commandname() const { return first; }
	void	commandname(const std::string& n) { first = n; }
	const std::string&	subcommandname() const { return second; }
	void	subcommandname(const std::string& n) { second = n; }
	std::string	toString() const;
};

std::ostream&	operator<<(std::ostream& out, const commandkey& key);

/**
 * \brief base class for command creators
 *
 * Command class instances are created by the commandcreator template.
 * This baseclass is needed so that we can use a smart pointer to the
 * command creators in the commandmap in the factory.
 */ 
class commandcreatorbase {
public:
	virtual clicommandptr	get(commandfactory& factory) = 0;
};
typedef std::shared_ptr<commandcreatorbase>	commandcreatorptr;

/**
 * \brief creator template
 *
 * This template creates a command object of a given type
 */
template<typename commandclass>
class commandcreator : public commandcreatorbase {
public:
	virtual clicommandptr	get(commandfactory& factory) {
		return clicommandptr(new commandclass(factory));
	}
};

/**
 * \brief command factory
 *
 * The command factory maintains a map of command creators and creates command
 * instances on demand.
 */
class commandfactory {
	std::map<commandkey, commandcreatorptr>	commandmap;
	commandcreatorptr	getcreator(const std::string& commandname,
				const std::vector<std::string>& arguments);
public:
	commandfactory();
	clicommandptr	get(const std::string& commandname,
		const std::vector<std::string>& arguments);
	std::string	summary();
	std::string	help(const std::string& commandname,
		const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro


#endif /* _clicommand_h */
