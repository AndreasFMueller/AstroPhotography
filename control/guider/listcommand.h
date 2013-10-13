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
public:
	listcommand() : clicommand("list") { }
	~listcommand() { }
	void	operator()(const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _listcommand_h */
