/*
 * guiderportcommand.h -- commands for the guiderport command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _guiderportcommand_h
#define _guiderportcommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

/**
 * \brief Class for guiderport access
 */
class guiderportcommand : public clicommand {
	void	release(const std::string& guiderportid,
			const std::vector<std::string>& arguments);
	void	activate(const std::string& guiderportid,
			const std::vector<std::string>& arguments);
	void	assign(const std::string& guiderportid,
			const std::vector<std::string>& arguments);
public:
	guiderportcommand(commandfactory& factory)
		: clicommand(factory, std::string("guiderport")) { }

	virtual void	operator()(const std::string& commandname,
		const std::vector<std::string>& arguments);

	virtual std::string	help() const;
	virtual std::string	summary() const;
};

} // namespace cli
} // namespace astro

#endif /* _guiderportcommand_h */
