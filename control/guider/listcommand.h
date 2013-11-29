/*
 * listcommand.h -- command class for list commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _listcommand_h
#define _listcommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

class listcommand : public clicommand {
	void	listmodules();
	void	listimages();
public:
	listcommand(commandfactory& factory) : clicommand(factory, "list") { }
	~listcommand() { }
	virtual void	operator()(const std::string& command,
			const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _listcommand_h */
