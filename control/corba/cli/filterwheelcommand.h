/*
 * filterwheelcommand.h -- commands for the filterwheel
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _filterwheelcommand_h
#define _filterwheelcommand_h

#include <clicommand.h>
#include <Filterwheels.h>

namespace astro {
namespace cli {

class filterwheelcommand : public clicommand {
	void	release(const std::string& filterwheelid,
			const std::vector<std::string>& arguments);
	void	info(FilterwheelWrapper& filterwheel,
			const std::vector<std::string>& arguments);
	void	assign(const std::string& filterwheelid,
			const std::vector<std::string>& arguments);
	void	position(FilterwheelWrapper& filterwheel,
			const std::vector<std::string>& arguments);
	void	wait(FilterwheelWrapper& filterwheel,
			const std::vector<std::string>& arguments);
public:
	filterwheelcommand(commandfactory& factory)
		: clicommand(factory, std::string("filterwheel")) { }

	virtual void	operator()(const std::string& commandname,
		const std::vector<std::string>& arguments);
	virtual std::string	help() const;
	virtual std::string	summary() const;
};

} // namespace astro
} // namespace cli

#endif /* _filterwheelcommand_h */
