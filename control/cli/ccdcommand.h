/*
 * ccdcommand.h -- commands for the ccd
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ccdcommand_h
#define _ccdcommand_H

#include <clicommand.h>
#include <Ccds.h>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::Ccd>	CcdWrapper;

class ccdcommand : public clicommand {
	void	release(const std::string& ccdid,
			const std::vector<std::string>& arguments);
	void	assign(const std::string& ccdid,
			const std::vector<std::string>& arguments);
	void	info(CcdWrapper& ccd,
			const std::vector<std::string>& arguments);
	void	start(CcdWrapper& ccd,
			const std::vector<std::string>& arguments);
	void	cancel(CcdWrapper& ccd,
			const std::vector<std::string>& arguments);
	void	wait(CcdWrapper& ccd,
			const std::vector<std::string>& arguments);
	void	image(CcdWrapper& ccd,
			const std::vector<std::string>& argumetns);
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
