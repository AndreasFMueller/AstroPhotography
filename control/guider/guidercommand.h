/*
 * guidercommand.h -- access to the guider
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _guidercommand_h
#define _guidercommand_h

#include <clicommand.h>
#include <Guiders.h>

namespace astro {
namespace cli {

class guidercommand : public clicommand {
	void	info(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);
	void	exposure(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);

	void	exposuretime(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);
	void	binning(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);
	void	size(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);
	void	offset(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);

	void	star(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);

	void	calibrate(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);
	void	wait(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);
	void	calibration(GuiderWrapper& guider,
			const std::vector<std::string>& arguments);
public:
	guidercommand(commandfactory& factory)
		: clicommand(factory, "guider") { }
	~guidercommand() { }
	virtual void	operator()(const std::string& command,
			const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _guidercommand_h */
