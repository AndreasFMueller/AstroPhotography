/*
 * ImageDirectory.h -- directory containing images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageDirectory_h
#define _ImageDirectory_h

#include <string>
#include <list>
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief Server diretory containing images
 *
 * The ImageDirectory is a singleton where image files are stored.
 * Images are identified by a string id, which can be any valid file
 * name. 
 */
class ImageDirectory {
	static std::string	_basedir;
public:
	static const std::string&	basedir() { return _basedir; }
	static void	basedir(const std::string& b) { _basedir = b; }
public:
	ImageDirectory() { }
	std::string	fullname(const std::string& filename) const;
	bool	isFile(const std::string& filename) const;
	long	fileSize(const std::string& filename) const;
	long	fileAge(const std::string& filename) const;
	std::list<std::string>	fileList() const;
	std::string	save(astro::image::ImagePtr image);
	void	remove(const std::string& filename);
	ImagePtr	getImagePtr(const std::string& filename);
};

} // namespace image
} // namespace astro

#endif /* _ImageDirectory_h */