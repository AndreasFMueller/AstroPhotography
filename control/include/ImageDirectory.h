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
#include <typeindex>

namespace astro {
namespace image {

/**
 * \brief Server directory containing images
 *
 * The ImageDirectory is a singleton where image files are stored.
 * Images are identified by a string id, which can be any valid file
 * name.  Hidden files, i.e. file names beginning with a dot, are
 * also invisible to the ImageDirectory.
 */
class ImageDirectory {
	static std::string	_basedir;
public:
	static const std::string&	basedir() { return _basedir; }
	static void	basedir(const std::string& b);
public:
	ImageDirectory() { }
	std::string	fullname(const std::string& filename) const;
	bool	isFile(const std::string& filename) const;
	long	fileSize(const std::string& filename) const;
	long	fileAge(const std::string& filename) const;
	int	bytesPerPixel(const std::string& filename) const;
	int	bytesPerPlane(const std::string& filename) const;
	std::type_index	pixelType(const std::string& filename) const;
	virtual std::list<std::string>	fileList();
	virtual std::string	save(astro::image::ImagePtr image);
protected:
	virtual void	write(astro::image::ImagePtr image,
				const std::string& filename);
public:
	virtual void	remove(const std::string& filename);
	ImagePtr	getImagePtr(const std::string& filename);
	void	setMetadata(const std::string& filename,
				const ImageMetadata& metadata);
	Metavalue	getMetadata(const std::string& filename,
				const std::string& keyword);
};

/**
 * \brief Server directory containing image with image database
 */
class ImageDatabaseDirectory : public ImageDirectory {
	static astro::persistence::Database	_database;
public:
	ImageDatabaseDirectory();
	virtual void	remove(const std::string& filename);
	virtual std::string	save(ImagePtr image);
	virtual std::list<std::string>	fileList();
protected:
	virtual void	write(astro::image::ImagePtr image,
				const std::string& filename);
	void	writeMetadata(long id, astro::image::ImagePtr image);
};

} // namespace image
} // namespace astro

#endif /* _ImageDirectory_h */
