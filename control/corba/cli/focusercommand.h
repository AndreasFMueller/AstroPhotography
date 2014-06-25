/*
 * focusercommand.h -- commands for the focuser
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _focusercommand_h
#define _focusercommand_h

#include <clicommand.h>
#include <Focusers.h>

namespace astro {
namespace cli {

class focusercommand : public clicommand {
	void	release(const std::string& focuserid,
			const std::vector<std::string>& arguments);
	void	assign(const std::string& focuserid,
			const std::vector<std::string>& arguments);
	void	info(FocuserWrapper& focuser,
			const std::vector<std::string>& arguments);
	void	set(FocuserWrapper& focuser,
			const std::vector<std::string>& arguments);
public:
	focusercommand(commandfactory& factory)
		: clicommand(factory, std::string("focuser")) { }

	virtual void	operator()(const std::string& commandname,
		const std::vector<std::string>& arguments);
	virtual std::string	help() const;
	virtual std::string	summary() const;
};

} // namespace astro
} // namespace cli

#endif /* _focusercommand_h */
