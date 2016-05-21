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
#include <AstroPersistence.h>

namespace astro {
namespace image {

/**
 * \brief Server directory containing images
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
	int	bytesPerPixel(const std::string& filename) const;
	virtual std::list<std::string>	fileList() const;
	virtual std::string	save(astro::image::ImagePtr image);
	virtual void	remove(const std::string& filename);
	ImagePtr	getImagePtr(const std::string& filename);
};

/**
 * \brief Server directory containing image with image database
 */
class ImageDatabaseDirectory : public ImageDirectory {
	static astro::persistence::Database	_database;
public:
	static astro::persistence::Database	database() { return _database; }
	static void	database(astro::persistence::Database db) {
		_database = db;
	}
public:
	ImageDatabaseDirectory() { }
	virtual void	remove(const std::string& filename);
	virtual std::string	save(ImagePtr image);
};

} // namespace image
} // namespace astro

#endif /* _ImageDirectory_h */
