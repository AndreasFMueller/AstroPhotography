/*
 * AstroProject.h -- project management and data archiving
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AstroProject_h
#define _AstroProject_h

#include <AstroImage.h>
#include <AstroPersistence.h>
#include <AstroCamera.h>

namespace astro {
namespace project {

/**
 * \brief Object to specify a set of images
 *
 * This class encapsulates the most important attributes, in particular
 * the attributes relevant to building dark and flat images.
 */
class ImageSpec {
private:
	astro::camera::Exposure::purpose_t	_purpose;
public:
	astro::camera::Exposure::purpose_t	purpose() const { return _purpose; }
	void	purpose(astro::camera::Exposure::purpose_t p) { _purpose = p; }

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
private:
	UUID	_uuid;
public:
	const UUID&	uuid() const { return _uuid; }
	void	uuid(const UUID& u) { _uuid = u; }

	// constructor
	ImageEnvelope(long id);
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

	// binning
private:
	astro::image::Binning	_binning;
public:
	const astro::image::Binning	binning() const { return _binning; }
	void	binning(const astro::image::Binning& b) { _binning = b; }

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

	// purpose
private:
	astro::camera::Exposure::purpose_t	_purpose;
public:
	astro::camera::Exposure::purpose_t	purpose() const { return _purpose; }
	void	purpose(astro::camera::Exposure::purpose_t c) { _purpose = c; }

private:
	std::string	_filter;
public:
	const std::string	filter() const { return _filter; }
	void	filter(const std::string& f) { _filter = f; }

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
	std::string	_name;
	astro::persistence::Database	_database;
	std::string	_directory;
	long	id(const std::string& filename);
	void	scan_directory(bool recurse = false);
	void	scan_recursive();
	void	scan_file(const std::string& filename);
	void	update_filename(long id, const std::string& filename);
public:
	ImageRepo(const std::string& name,
		astro::persistence::Database database,
		const std::string& directory, bool scan = false);
	const std::string&	name() const { return _name; }
	bool	has(long id);
	bool	has(const UUID& uuid);
	std::string	filename(long id);
	std::string	pathname(long id);
	long	getId(const UUID& uuid);
	astro::image::ImagePtr	getImage(long id);
	astro::image::ImagePtr	getImage(const UUID& uuid);
	ImageEnvelope	getEnvelope(long id);
	ImageEnvelope	getEnvelope(const UUID& uuid);

	long	save(astro::image::ImagePtr image);
	void	remove(long id);
	void	remove(const UUID& uuid);

	std::set<ImageEnvelope>	get(const ImageSpec& spec);
	std::set<UUID>	getUUIDs(const std::string& condition);
	std::vector<int>	getIds();
	std::vector<int>	getIds(const std::string& condition);
	std::vector<std::string>	getProjectnames();
};
typedef std::shared_ptr<ImageRepo>	ImageRepoPtr;

/**
 * \brief A class that implements a replication from one repo to another
 */
class RepoReplicator {
	std::set<long>	uuid2ids(ImageRepoPtr repo, const std::set<UUID>& uuids);
public:
	RepoReplicator();
	int	replicate(ImageRepoPtr from, ImageRepoPtr to,
			bool remove = false);
	int	synchronize(ImageRepoPtr repo1, ImageRepoPtr repo2);
};

/**
 * \brief A class describing an image repository
 */
class ImageRepoInfo {
public:
	std::string	reponame;
	std::string	database;
	std::string	directory;
	bool	hidden;
	bool	operator==(const ImageRepoInfo& other) const;
};

/**
 * \brief Project part
 */
class Part {
private:
	long	_partno;
public:
	long	partno() const { return _partno; }
	void	partno(long p) { _partno = p; }
	bool	operator<(const Part& other) const {
		return _partno < other._partno;
	}
private:
	std::string	_instrument;
public:
	const std::string&	instrument() const { return _instrument; }
	void	instrument(const std::string& i) { _instrument = i; }

private:
	astro::camera::Exposure	_exposure;
public:
	astro::camera::Exposure	exposure() const { return _exposure; }
	void	exposure(const astro::camera::Exposure e) { _exposure = e; }

private:
	std::string	_filtername;
public:
	std::string	filtername() const { return _filtername; }
	void	filtername(const std::string& f) { _filtername = f; }

private:
	float	_temperature;
public:
	float	temperature() const { return _temperature; }
	void	temperature(float t) { _temperature = t; }

private:
	std::string	_taskserver;
public:
	std::string	taskserver() const { return _taskserver; }
	void	taskserver(const std::string& u) { _taskserver = u; }

private:
	long	_taskid;
public:
	long	taskid() const { return _taskid; }
	void	taskid(long t) { _taskid = t; }

private:
	long	_repoid;
public:
	long	repoid() const { return _repoid; }
	void	repoid(long r) { _repoid = r; }

	Part() {
		_partno = -1;
		_taskid = -1;
		_repoid = -1;
	}
};
typedef std::shared_ptr<Part>	PartPtr;

/**
 * \brief Project information
 */
class Project {
	std::string	_name;
	std::string	_description;
	std::string	_object;
	std::string	_repository;
	time_t	_started;
public:
	Project(const std::string& name);

	const std::string&	name() const { return _name; }

	const std::string&	description() const { return _description; }
	void	description(const std::string& d) { _description = d; }

	const std::string	object() const { return _object; }
	void	object(const std::string& o) { _object = o; }

	const std::string	repository() const { return _repository; }
	void	repository(const std::string& r) { _repository = r; }

	time_t	started() const { return _started; }
	void	started(time_t s) { _started = s; }

	std::map<long, PartPtr>	parts;
	PartPtr	part(long partno) const;
	void	add(PartPtr part);
};

} // namespace project
} // namespace astro

#endif /* _AstroProject_h */
