/*
 * ccdcommand.h -- commands for the ccd
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ccdcommand_h
#define _ccdcommand_H

#include <clicommand.h>

namespace astro {
namespace cli {

class ccdcommand : public clicommand {
	void	release(const std::string& ccdid,
			const std::vector<std::string>& arguments);
	void	info(const std::string& ccdid,
			const std::vector<std::string>& arguments);
	void	assign(const std::string& ccdid,
			const std::vector<std::string>& arguments);
public:
	ccdcommand(commandfactory& factory)
		: clicommand(factory, std::string("ccd")) { }

	virtual void	operator()(const std::string& commandname,
		const std::vector<std::string>& arguments);
	virtual std::string	help() const;
	virtual std::string	summary() const;
};

} // namespace astro
} // namespace cli

#endif /* _ccdcommand_h */
