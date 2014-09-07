/*
 * AstroProject.h -- project management and data archiving
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroProject_h
#define _AstroProject_h

#include <AstroImage.h>
#include <AstroPersistence.h>

namespace astro {
namespace project {

/**
 * \brief Object to specify a set of images
 *
 * This class encapsulates the most important attributes, in particular
 * the attributes relevant to building dark and flat images.
 */
class ImageSpec {
public:
	typedef enum { dark = 1, flat = 2, light = 0 } category_t;
private:
	category_t	_category;
public:
	category_t	category() const { return _category; }
	void	category(category_t c) { _category = c; }

	// name of the camera that took the image
private:
	std::string	_cameraname;
public:
	const std::string&	cameraname() const { return _cameraname; }
	void	cameraname(const std::string& c) { _cameraname = c; }

	// exposure time of the image
private:
	float	_exposuretime;
public:
	float	exposuretime() const { return _exposuretime; }
	void	exposuretime(float e) { _exposuretime = e; }

	// temperature of the CCD chip
private:
	float	_temperature;
public:
	float	temperature() const { return _temperature; }
	void	temperature(float t) { _temperature = t; }

	// name of the projecct
private:
	std::string	_project;
public:
	const std::string&	project() const { return _project; }
	void	project(const std::string& p) { _project = p; }
};

/**
 * \brief An object containing anything but the image itself
 *
 * The ImageServer can find ImageEnvelope objects, but it can also be used
 * to request the image itself.
 */
class ImageEnvelope {
	long	_id;
public:
	long	id() const { return _id; }
	operator	long() const { return _id; }
private:
	astro::image::ImageMetadata	metadata;
	astro::image::ImageSize	_size;
public:
	ImageEnvelope(const astro::image::ImagePtr image);

	std::string	cameraname() const;
	float	exposuretime() const;
	float	temperature() const;
	const astro::image::ImageSize&	size() const { return _size; }
	ImageSpec::category_t	category() const;
	std::string	project() const;

	const astro::image::Metavalue&	getMetadata(const std::string& keyword) const;
	friend class ImageServer;
};

/**
 * \brief A server for images
 *
 * The image server is an interface to retrieve images identified either
 * by an id or by some attributes collected in the ImageSpec class.
 */
class ImageServer {
	astro::persistence::Database	_database;
	std::string	_directory;
	long	id(const std::string& filename);
	void	scan_directory(bool recurse = false);
	void	scan_recursive();
	void	scan_file(const std::string& filename);
public:
	ImageServer(astro::persistence::Database database,
		const std::string& directory);

	std::string	filename(long id);
	astro::image::ImagePtr	getImage(long id);
	ImageEnvelope	getEnvelope(long id);

	long	save(astro::image::ImagePtr image);
	void	remove(long id);

	std::set<ImageEnvelope>	get(const ImageSpec& spec);
};


} // namespace project
} // namespace astro

#endif /* _AstroProject_h */
