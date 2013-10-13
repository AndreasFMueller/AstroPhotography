/*
 * locatorcommand.h -- locator related commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _locatorcommand_h
#define _locatorcommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

class locatorcommand : public clicommand {
public:
	locatorcommand() : clicommand(std::string("locator")) { }
	void	operator()(const std::string& command,
			const std::vector<std::string>& arguments);
};

} // namespace cli
} // namespace astro

#endif /* _locatorcommand_h */
