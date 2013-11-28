/*
 * ImageDirectory.h -- directory containing images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageDirectory_h
#define _ImageDirectory_h

#include <string>
#include <list>
#include <image.hh>
#include <AstroImage.h>

namespace Astro {

class ImageDirectory {
	static std::string	_basedir;
public:
	static const std::string&	basedir() { return _basedir; }
	static void	basedir(const std::string& b) { _basedir = b; }
public:
	ImageDirectory() { }
	std::string	fullname(const std::string& filename) const;
	bool	isFile(const std::string& filename);
	long	fileSize(const std::string& filename);
	long	fileAge(const std::string& filename);
	std::list<std::string>	fileList();
	std::string	save(astro::image::ImagePtr image);
	Image_ptr	getImage(const std::string& filename);
};

} // namespace Astro

#endif /* _ImageDirectory_h */
