/*
 * cameracommand.h -- commands for the camera command
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _cameracommand_h
#define _cameracommand_h

#include <clicommand.h>

namespace astro {
namespace cli {

/**
 * \brief Class for camera access
 */
class cameracommand : public clicommand {
	void	release(const std::string& cameraid,
			const std::vector<std::string>& arguments);
	void	info(const std::string& cameraid,
			const std::vector<std::string>& arguments);
	void	assign(const std::string& cameraid,
			const std::vector<std::string>& arguments);
public:
	cameracommand(commandfactory& factory)
		: clicommand(factory, std::string("camera")) { }

	virtual void	operator()(const std::string& commandname,
		const std::vector<std::string>& arguments);

	virtual std::string	help() const;
	virtual std::string	summary() const;
};

} // namespace cli
} // namespace astro

#endif /* _cameracommand_h */
