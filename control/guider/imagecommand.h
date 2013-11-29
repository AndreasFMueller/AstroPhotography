/*
 * imagecommand.h -- access to images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _imagecommand_h
#define _imagecommand_h

#include <ObjWrapper.h>

namespace astro {
namespace cli {

typedef ObjWrapper<Astro::Image> ImageWrapper;

class imagecommand : public clicommand {
	void	assign(const std::string& imageid,
			const std::vector<std::string>& filename);
	void	release(const std::string& imageid);
	void	info(ImageWrapper& image);
	void	save(ImageWrapper& image, const std::string& filename);
	void	remove(ImageWrapper& image);
public:
	imagecommand(commandfactory& factory) : clicommand(factory, "images") { }
	virtual void	operator()(const std::string& command,
				const std::vector<std::string>& arguments);
	virtual std::string	summary() const;
	virtual std::string	help() const;
};

} // namespace cli
} // namespace astro

#endif /* _imagecommand_h */
