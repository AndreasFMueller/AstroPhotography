/*
 * ImageRepo.cpp -- implementation of the image repository
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <includes.h>
#include "ImageRepoTables.h"
#include <numeric>

using namespace astro::persistence;
using namespace astro::image;
using namespace astro::io;
using namespace astro::camera;

namespace astro {
namespace project {

/**
 * \brief Create an image server
 */
ImageRepo::ImageRepo(const std::string& name, Database database,
	const std::string& directory, bool scan)
	: _name(name), _database(database), _directory(directory) {
	// 

	// scan the directory for 
	if (scan) {
		scan_directory();
	}
}

/**
 * \brief get the id of an image identified by its filename
 */
long	ImageRepo::id(const std::string& filename) {
	ImageTable	images(_database);
	return images.id(filename);
}

/**
 * \brief process a single file during a scan
 */
void	ImageRepo::scan_file(const std::string& filename) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scanning file '%s' (%d)",
		filename.c_str(), filename.length());
	// does the filename end in ".fits"?
	if (filename.size() < 5) {
		return;
	}
	std::string	extension = filename.substr(filename.length() - 5, 5);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "substr: '%s'", extension.c_str());
	if (extension != ".fits") {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s not a FITS file",
			filename.c_str());
		return;
	}

	// check whether this is a file
	std::string	fullname = _directory + "/" + filename;
	struct stat	sb;
	if (stat(fullname.c_str(), &sb) < 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot stat file %s: %s",
			fullname.c_str(), strerror(errno));
		return;
	}

	if (!S_ISREG(sb.st_mode)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s: not a regular file",
			fullname.c_str());
		return;
	}

	// find out whether the database already contains this filename
	ImageTable	images(_database);
	try {
		long	id = images.id(filename);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s exists with id %ld", 
			filename.c_str(), id);
		// if we get to this point, the file already exists, so we
		// skip it
		return;
	} catch (...) {
		// file does not exist
	}

	// read the metadata
        io::FITSinfileBase      infile(fullname);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "size: %s",
		infile.getSize().toString().c_str());

	// create an information record
	ImageRecord	imageinfo;
	imageinfo.filename = filename;
	imageinfo.project = "unknown";
	imageinfo.created = sb.st_ctime;
	try {
		imageinfo.camera
			= (std::string)infile.getMetadata("INSTRUME");
	} catch(...) { }
	imageinfo.width = infile.getSize().width();
	imageinfo.height = infile.getSize().height();
	imageinfo.xbin = 1;
	try {
		imageinfo.xbin
			= (int)infile.getMetadata("XBINNING");
	} catch(...) { }
	imageinfo.ybin = 1;
	try {
		imageinfo.ybin
			= (int)infile.getMetadata("YBINNING");
	} catch(...) { }
	imageinfo.depth = infile.getPlanes();
	imageinfo.pixeltype = infile.getPixeltype();
	imageinfo.exposuretime = 0;
	try {
		imageinfo.exposuretime
			= (double)infile.getMetadata("EXPTIME");
	} catch(...) { }
	imageinfo.temperature = 0;
	try {
		imageinfo.temperature
			= (double)infile.getMetadata("CCD-TEMP");
	} catch(...) { }
	imageinfo.purpose = "light";
	imageinfo.bayer = "    ";
	imageinfo.observation = "1970-01-01T00:00:00.000";
	imageinfo.uuid = "";
	try {
		imageinfo.uuid = (std::string)(infile.getMetadata("UUID"));
	} catch (...) { }

	// add the entry to the table
	long	imageid = images.add(imageinfo);

	// in the part below we need the metatdata table
	MetadataTable	metadatatable(_database);

	// get all the metadata from the infile
	int	seqno = 0;
	ImageMetadata	md = infile.getAllMetadata();

	// now add an entry for each meta data record
	ImageMetadata::const_iterator	mi;
	for (mi = md.begin(); mi != md.end(); mi++) {
		MetadataRecord	m(-1, imageid);
		m.seqno = seqno;
		m.key = mi->first;
		m.value = mi->second.getValue();
		m.comment = mi->second.getComment();
		metadatatable.add(m);
		seqno++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d metadata records added", seqno);
}

/**
 * \brief Scan a directory for images
 */
void	ImageRepo::scan_directory(bool recurse) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "scan directory %s", _directory.c_str());

	if (recurse) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"recursive scan not implemented");
		throw std::runtime_error("not implemented");
	}

	// set up a counter so we can count the number of files we find
	int	counter = 0;

	// open the directory
	DIR     *dir = opendir(_directory.c_str());
	if (NULL == dir) {
		throw std::runtime_error("cannot open directory");
	}
	struct dirent	*d;
	while (NULL != (d = readdir(dir))) {
		std::string     filename(d->d_name);
		scan_file(filename);
		counter++;
	}

	// close the directory
	closedir(dir);

	// report the number of files scanned
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d files scanned", counter);
}

/**
 * \brief Retrieve an image
 */
std::string	ImageRepo::filename(long id) {
	ImageTable	table(_database);
	if (id < 0) {
		id = table.lastid();
	}
	return ImageTable(_database).byid(id).filename;
}

std::string	ImageRepo::pathname(long id) {
	return _directory + "/" + filename(id);
}

/**
 * \brief Find out whether a given id is in the table
 */
bool    ImageRepo::has(long id) {
	ImageTable	images(_database);
	return images.exists(id);
}

/**
 * \brief Find out whether a given UUID exists in the table
 */
bool    ImageRepo::has(const UUID& uuid) {
	ImageTable	images(_database);
	std::string	condition = stringprintf("uuid = '%s'",
				((std::string)(uuid)).c_str());
	std::list<ImageRecord>	records = images.select(condition);
	return (records.size() > 0);
}

/**
 * \brief Find the id based on the UUID
 */
long	ImageRepo::getId(const UUID& uuid) {
	ImageTable	images(_database);
	std::string	condition = stringprintf("uuid = '%s'",
				((std::string)(uuid)).c_str());
	std::list<ImageRecord>	records = images.select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no image with uuid %s",
			((std::string)uuid).c_str());
		throw std::runtime_error(msg);
	}
	ImageRecord	imageinfo = *(records.begin());
	return imageinfo.id();
}

/**
 * \brief Get an image
 */
ImagePtr	ImageRepo::getImage(long id) {
	std::string	f = pathname(id);
	FITSin	in(f);
	return in.read();
}

ImagePtr	ImageRepo::getImage(const UUID& uuid) {
	return getImage(getId(uuid));
}

static ImageEnvelope	convert(const ImageRecord& imageinfo,
				MetadataTable& metadatatable) {
	ImageEnvelope	result(imageinfo.id());

	// image geometry
	result.size(ImageSize(imageinfo.width, imageinfo.height));
	result.binning(Binning(imageinfo.xbin, imageinfo.ybin));

	// retrieve all the metadata available
	std::string	condition = stringprintf("imageid = %ld",
					imageinfo.id());
	std::list<MetadataRecord>	mdrecords
		= metadatatable.select(condition);

	// convert the MetadataRecords into actual metadata
	std::list<MetadataRecord>::const_iterator	mi;
	for (mi = mdrecords.begin(); mi != mdrecords.end(); mi++) {
		Metavalue	m = FITSKeywords::meta(mi->key, mi->value,
					mi->comment);
		result.metadata.setMetadata(m);
	}

	// envelope variables
	result.filename(imageinfo.filename);
	result.project(imageinfo.project);
	result.created(imageinfo.created);
	result.camera(imageinfo.camera);
	result.exposuretime(imageinfo.exposuretime);
	result.temperature(imageinfo.temperature);
	result.purpose(Exposure::string2purpose(imageinfo.purpose));
	result.filter(imageinfo.filter);
	result.bayer(imageinfo.bayer);
	result.observation((time_t)FITSdate(imageinfo.observation));
	result.uuid(UUID(imageinfo.uuid));

	// we are done, return the envelope
	return result;
}

/**
 * \brief Retrieve the metadata for an image
 */
ImageEnvelope	ImageRepo::getEnvelope(long id) {
	// create a result record
	ImageEnvelope	result(id);

	// read the global information from the database
	ImageRecord	imageinfo = ImageTable(_database).byid(id);
	MetadataTable	metadatatable(_database);
	return convert(imageinfo, metadatatable);
}

/**
 * \brief Retrieve image information based on the uuid
 */
ImageEnvelope	ImageRepo::getEnvelope(const UUID& uuid) {
	ImageTable	images(_database);
	std::string	condition = stringprintf("uuid = '%s'",
				((std::string)(uuid)).c_str());
	std::list<ImageRecord>	records = images.select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no image with uuid %s",
			((std::string)uuid).c_str());
		throw std::runtime_error(msg);
	}
	ImageRecord	imageinfo = *(records.begin());
	MetadataTable	metadatatable(_database);
	return convert(imageinfo, metadatatable);
}

/**
 * \brief Save an image in the repository
 */
long	ImageRepo::save(ImagePtr image) {
	// if the image does not have a UUID yet, add one
	if (!image->hasMetadata("UUID")) {
		UUID	uuid;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new uuid: %s",
			((std::string)uuid).c_str());
		image->setMetadata(FITSKeywords::meta("UUID",
			(std::string)uuid));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image uuid: %s",
		((std::string)(image->getMetadata("UUID"))).c_str());


	// build imageinfo reocord
	long	imageid = -1;
	ImageRecord	imageinfo(imageid);
	try {
		imageinfo.project
			= (std::string)image->getMetadata("PROJECT");
	} catch (...) { }
	try {
		imageinfo.camera
			= (std::string)image->getMetadata("INSTRUME");
	} catch (...) { }
	imageinfo.width = image->size().width();
	imageinfo.height = image->size().height();
	try {
		imageinfo.xbin
			= (int)image->getMetadata("XBINNING");
	} catch (...) { }
	try {
		imageinfo.ybin
			= (int)image->getMetadata("YBINNING");
	} catch (...) { }
	imageinfo.depth = image->planes();
	imageinfo.pixeltype = image->bitsPerPlane();
	try {
		imageinfo.exposuretime
			= (double)image->getMetadata("EXPTIME");
	} catch (...) { }
	try {
		imageinfo.temperature
			= (double)image->getMetadata("CCD-TEMP");
	} catch (...) { }
	try {
		imageinfo.purpose
			= (std::string)image->getMetadata("PURPOSE");
	} catch (...) { }
	try {
		imageinfo.filter
			= (std::string)image->getMetadata("FILTER");
	} catch (...) { }
	try {
		imageinfo.bayer
			= (std::string)image->getMetadata("BAYER");
	} catch (...) { }
	try {
		imageinfo.observation
			= (std::string)image->getMetadata("DATE-OBS");
	} catch (...) { }
	try {
		imageinfo.uuid
			= (std::string)image->getMetadata("UUID");
	} catch (...) { }

	// begin a transaction in the database
	_database->begin("saveimage");

	// now try to save the image info record
	try {
		// save the image info
		ImageTable	images(_database);
		imageid = images.add(imageinfo);

		// write the metadata to the metadata tabe
		MetadataTable	metadata(_database);
		
		// now add an entry for each meta data record
		ImageMetadata::const_iterator	mi;
		long	seqno = 0;
		for (mi = image->begin(); mi != image->end(); mi++) {
			MetadataRecord	m(-1, imageid);
			m.seqno = seqno;
			m.key = mi->first;
			m.value = mi->second.getValue();
			m.comment = mi->second.getComment();
			metadata.add(m);
			seqno++;
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%d metadata records added",
			seqno);

		// first we have to create a file name for the image
		std::string	fullname
			= stringprintf("%s/image-%s-%05ld.fits",
				_directory.c_str(), _name.c_str(), imageid);
		std::string	filename
			= fullname.substr(_directory.size() + 1);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "full name: %s",
			fullname.c_str());

		// write the image
		unlink(fullname.c_str());
		try {
			FITSout	out(fullname);
			out.write(image);
			debug(LOG_DEBUG, DEBUG_LOG, 0, "image written to %s",
				fullname.c_str());
		} catch (...) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "writing the image to "
				"'%s' failed, cleaning up", fullname.c_str());
			unlink(fullname.c_str());
			throw;
		}

		// now we have to update the filename, which only became known
		// when the ID became known from the add operation
		update_filename(imageid, filename);

		// commit the transaction, only at this point do the database
		// entries become persistent. This ensures that information
		// about the image only becomes visible in the database when
		// the image file has actually been written to disk
		_database->commit("saveimage");
		debug(LOG_DEBUG, DEBUG_LOG, 0, "image %ld committed as '%s'",
			imageid, filename.c_str());
	} catch (...) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"adding image failed, rolling back");
		// if anything fails, then we roll back the transaction, so
		// the database will be clean again. In particular, if the
		// disk write fails, nothing will show up in the database.
		_database->rollback("saveimage");
		throw;
	}


	// done
	return imageid;
}

/**
 * \brief Remove the image and the metadata from the database
 */
void	ImageRepo::remove(long id) {
	_database->begin();
	try {
		// first get the path name from the database
		std::string	fullname = pathname(id);

		// now we have the information that we need, so we can
		// remove all other traces from the database. Since we
		// are in a transaction, nothing is actually lost until
		// we commit the transaction
		ImageTable	images(_database);
		images.remove(id);

		// now remove the image file
		if (unlink(fullname.c_str())) {
			std::string	msg = stringprintf("cannot remove "
				"image '%s': %s",
				fullname.c_str(), strerror(errno));
			debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
			throw std::runtime_error(msg);
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"image file '%s' successfully removed",
			fullname.c_str());

		// ok, if we get to this point, then the image was removed,
		// and we have to commit to ensure that the database is
		// consistent with the disk
		_database->commit();
	} catch (...) {
		// Oops, there was a problem removing the file. We better roll
		// back the transaction to ensure that the database is still
		// consistent with the on disk storage
		debug(LOG_DEBUG, DEBUG_LOG, 0, "error during remove, "
			"rolling back");
		_database->rollback();
		throw;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image %ld removed from repository", id);
}

/**
 * \brief Remove an image based on the uuid
 */
void	ImageRepo::remove(const UUID& uuid) {
	remove(getId(uuid));
}

static float	temperature_min(float temperature) {
	return 0.99 * (273.15 + temperature) - 273.15;
}

static float	temperature_max(float temperature) {
	return 1.01 * (273.15 + temperature) - 273.15;
}

class condition : public std::string {
public:
	condition(const std::string& s) : std::string(s) { }

	condition	operator+(const condition& other) {
		if ((size() == 0) && (other.size() == 0)) {
			return std::string("");
		}
		if (size() == 0) {
			return other;
		}
		if (other.size() == 0) {
			return *this;
		}
		return condition("(" + *this + ") and (" + other + ")");
	}
};

/**
 * \brief get a set of images matching the specifcation
 */
std::set<ImageEnvelope>	ImageRepo::get(const ImageSpec& spec) {
	std::list<condition>	conditions;
	// add purpose condition
	if (spec.purpose() >= 0) {
		conditions.push_back(condition(stringprintf("purpose = '%s'",
			Exposure::purpose2string(spec.purpose()).c_str())));
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "conditions so far: %d",
		conditions.size());

	// add cameraname condition
	if (spec.camera().size() > 0) {
		conditions.push_back(condition(stringprintf("cameraname = '%s'",
			spec.camera().c_str())));
	}

	// add exposure time condition
	if (spec.exposuretime() > 0) {
		conditions.push_back(condition(stringprintf(
			"%f <= exposuretime and exposuretime <= %f",
			spec.exposuretime() * 0.9, spec.exposuretime() * 1.1)));
	}

	// add temperature condition
	if (spec.temperature() > -273.15) {
		conditions.push_back(condition(stringprintf(
			"%f <= temperature and temperature < %f",
			temperature_min(spec.temperature()),
			temperature_max(spec.temperature()))));
	}

	// add project condition
	if (spec.project().size() > 0) {
		conditions.push_back(condition(stringprintf(
			"project = '%s'", spec.project().c_str())));
	}

	// if we have no conditions so far, build an empty condition
	if (0 == conditions.size()) {
		conditions.push_back(condition("0 = 0"));
	}

	// concatenate the conditions
	condition	all =  std::accumulate(conditions.begin(),
					conditions.end(), condition(""));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "accumulated condition %s", all.c_str());

	// query the database 
	ImageTable	imagetable(_database);
	MetadataTable	metadatatable(_database);

	std::list<ImageRecord>	images = imagetable.select(all);

	// build the result set
	std::set<ImageEnvelope>	resultset;
	std::list<ImageRecord>::const_iterator	ii;
	for (ii = images.begin(); ii != images.end(); ii++) {
		resultset.insert(convert(*ii, metadatatable));
	}
	return resultset;
}

/**
 * \brief update the filename of an entry
 */
void	ImageRepo::update_filename(long id, const std::string& filename) {
	ImageTable	imagetable(_database);
	UpdateSpec	updatespec;
	FieldValueFactory	factory;
	updatespec.insert(std::make_pair(std::string("filename"),
		factory.get(filename)));
	imagetable.updaterow(id, updatespec);
}

/**
 * \brief get the set of all uuids from the imagetable
 */
std::set<UUID>	ImageRepo::getUUIDs(const std::string& condition) {
	ImageTable	imagetable(_database);
	std::list<ImageRecord>	images = imagetable.select(condition);
	std::set<UUID>	result;
	for (auto ptr = images.begin(); ptr != images.end(); ptr++) {
		result.insert(ptr->uuid);
	}
	return result;
}

std::vector<int>	ImageRepo::getIds() {
	return getIds("0 = 0");
}

std::vector<int>	ImageRepo::getIds(const std::string& condition) {
	ImageTable	imagetable(_database);
	std::list<ImageRecord>	images = imagetable.select(condition);
	std::vector<int>	result;
	for (auto ptr = images.begin(); ptr != images.end(); ptr++) {
		result.push_back(ptr->id());
	}
	return result;
}

std::vector<std::string>	ImageRepo::getProjectnames() {
	std::vector<std::string>	result;
	std::string	query("select distinct project from images order by 1");
	Result	res = _database->query(query);
	for (auto ptr = res.begin(); ptr != res.end(); ptr++) {
		std::string	projectname = (*ptr)[0]->stringValue();
		result.push_back(projectname);
	}
	return result;
}

} // namespace project
} // namespace astro
