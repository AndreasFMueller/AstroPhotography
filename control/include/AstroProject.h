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
	typedef enum { light = 0, dark = 1, flat = 2 } category_t;
private:
	category_t	_category;
public:
	category_t	category() const { return _category; }
	void	category(category_t c) { _category = c; }

	// name of the camera that took the image
private:
	std::string	_camera;
public:
	const std::string&	camera() const { return _camera; }
	void	camera(const std::string& c) { _camera = c; }

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

	// constructor
	ImageSpec();
};

/**
 * \brief An object containing anything but the image itself
 *
 * The ImageRepo can find ImageEnvelope objects, but it can also be used
 * to request the image itself.
 */
class ImageEnvelope {
private:
	long	_id;
public:
	long	id() const { return _id; }
	void	id(long l) { _id = l; }
	operator	long() const { return _id; }

	// constructor
	ImageEnvelope(long id) : _id(id) { }
	ImageEnvelope(const astro::image::ImagePtr image);

	// filename
private:
	std::string	_filename;
public:
	const std::string&	filename() const { return _filename; }
	void	filename(const std::string& f) { _filename = f; }

	// project
private:
	std::string	_project;
public:
	const std::string&	project() const { return _project; }
	void	project(const std::string& p) { _project = p; }

	// created
private:
	time_t	_created;
public:
	time_t	created() const { return _created; }
	void	created(time_t c) { _created = c; }

	// camera
private:
	std::string	_camera;
public:
	const std::string&	camera() const { return _camera; }
	void	camera(const std::string& c) { _camera = c; }

	// size
private:
	astro::image::ImageSize	_size;
public:
	const astro::image::ImageSize&	size() const { return _size; }
	void	size(const astro::image::ImageSize& s) { _size = s; }

	// exposuretime
private:
	float	_exposuretime;
public:
	float	exposuretime() const { return _exposuretime; }
	void	exposuretime(float e) { _exposuretime = e; }

	// temperature
private:
	float	_temperature;
public:
	float	temperature() const { return _temperature; }
	void	temperature(float t) { _temperature = t; }

	// category
private:
	ImageSpec::category_t	_category;
public:
	ImageSpec::category_t	category() const { return _category; }
	void	category(ImageSpec::category_t c) { _category = c; }

	// bayer
private:
	std::string	_bayer;
public:
	const std::string&	bayer() const { return _bayer; }
	void	bayer(const std::string& b) { _bayer = b; }

private:
	time_t	_observation;
public:
	time_t	observation() const { return _observation; }
	void	observation(time_t o) { _observation = o; }

	astro::image::ImageMetadata	metadata;
	const astro::image::Metavalue&	getMetadata(const std::string& keyword) const;
	friend class ImageRepo;
	std::string	toString() const;
};

/**
 * \brief A server for images
 *
 * The image server is an interface to retrieve images identified either
 * by an id or by some attributes collected in the ImageSpec class.
 */
class ImageRepo {
	astro::persistence::Database	_database;
	std::string	_directory;
	long	id(const std::string& filename);
	void	scan_directory(bool recurse = false);
	void	scan_recursive();
	void	scan_file(const std::string& filename);
public:
	ImageRepo(astro::persistence::Database database,
		const std::string& directory, bool scan = true);

	std::string	filename(long id);
	std::string	pathname(long id);
	astro::image::ImagePtr	getImage(long id);
	ImageEnvelope	getEnvelope(long id);

	long	save(astro::image::ImagePtr image);
	void	remove(long id);

	std::set<ImageEnvelope>	get(const ImageSpec& spec);
};

/**
 * \brief A class describing an image repository
 */
class ImageRepoInfo {
public:
	std::string	reponame;
	std::string	database;
	std::string	directory;
	bool	operator==(const ImageRepoInfo& other) const;
};


} // namespace project
} // namespace astro

#endif /* _AstroProject_h */
