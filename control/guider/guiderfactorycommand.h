/*
 * guiderfactorycommand.h -- access to the guider factory
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _guiderfactorycommand_h
#define _guiderfactorycommand_h

#include <clicommand.h>
#include <Guiders.h>

namespace astro {
namespace cli {

class guiderfactorycommand : public clicommand {
	void	assign(const std::string& guiderid,
			const std::vector<std::string>& arguments);
	void	release(const std::string& guiderid,
			const std::vector<std::string>& arguments);
public:
	guiderfactorycommand(commandfactory& factory)
		: clicommand(factory, "guiderfactory") { }
	~guiderfactorycommand() { }
	virtual void	operator()(const std::string& command,
			const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _guiderfactorycommand_h */
