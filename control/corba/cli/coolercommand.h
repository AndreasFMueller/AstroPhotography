/*
 * coolercommand.h -- commands for the cooler
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _coolercommand_h
#define _coolercommand_h

#include <clicommand.h>
#include <camera.hh>
#include <ObjWrapper.h>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::Cooler>	CoolerWrapper;

class coolercommand : public clicommand {
	CoolerWrapper	getCooler(const std::string& ccdid);
	void	set(CoolerWrapper& cooler,
			const std::vector<std::string>& arguments);
	void	status(CoolerWrapper& cooler,
			const std::vector<std::string>& arguments);
	void	enable(CoolerWrapper& cooler,
			const std::vector<std::string>& arguments);
	void	disable(CoolerWrapper& cooler,
			const std::vector<std::string>& arguments);
	void	waitfor(CoolerWrapper& cooler,
			const std::vector<std::string>& arguments);
public:
	coolercommand(commandfactory& factory)
		: clicommand(factory, std::string("cooler")) { }

	virtual void	operator()(const std::string& commandname,
		const std::vector<std::string>& arguments);
	virtual std::string	help() const;
	virtual std::string	summary() const;
};

} // namespace astro
} // namespace cli

#endif /* _coolercommand_h */
