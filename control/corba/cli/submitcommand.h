/*
 * submitcommand.h -- declaration of the submit command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _submitcommand_h
#define _submitcommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

class submitcommand : public clicommand {
public:
	submitcommand(commandfactory& factory) : clicommand(factory, "submit") { }
	~submitcommand() { }
	virtual void	operator()(const std::string& command,
				const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _submitcommand_h */
