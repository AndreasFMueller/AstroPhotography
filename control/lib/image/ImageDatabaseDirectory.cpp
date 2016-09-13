/*
 * ImageDatabaseDirectory.cpp -- database backed image directory implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageDirectory.h>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <ImagePersistence.h>

namespace astro {
namespace image {

persistence::Database	ImageDatabaseDirectory::_database;

/**
 * \brief Constructor for the database directory
 */
ImageDatabaseDirectory::ImageDatabaseDirectory() {
	_database = astro::persistence::DatabaseFactory::get(fullname(".files.db"));
}

/**
 * \brief retrieve file names from database instead of the directory
 *
 * This may be more efficient
 */
std::list<std::string>	ImageDatabaseDirectory::fileList() {
	std::string	q("select filename from images");
	astro::persistence::Result	rows = _database->query(q);
	std::list<std::string>	result;
	for (auto ptr = rows.begin(); ptr != rows.end(); ptr++) {
		astro::persistence::Row	row = *ptr;
		std::string	filename = row[0]->toString();
		result.push_back(filename);
	}
	return result;
}

/**
 * \brief Remove an image from the directory and the database
 */
void	ImageDatabaseDirectory::remove(const std::string& filename) {
	// remove the image from the directory
	try {
		ImageDirectory::remove(filename);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error while removing file %s",
			filename.c_str());
	}

	// if the database is not set, just ignore it, fall back to the 
	// behaviour of the ImageDirectory
	if (!_database) {
		debug(LOG_WARNING, DEBUG_LOG, 0, "warning: no database");
		return;
	}

	// now remove the image also from the database
	_database->begin();
	try {
		ImageTable	imagetable(_database);
		std::string	condition = stringprintf("filename = '%s'",
			filename.c_str());
		std::list<long>	idlist = imagetable.selectids(condition);
		if (idlist.size() != 1) {
			return;
		}

		// get the id
		long	id = idlist.front();
		
		// remove the entry
		imagetable.remove(id);

		// remove all related entries from the attribute table
		ImageAttributeTable	attributetable(_database);
		condition = stringprintf("image = %d", id);
		idlist = attributetable.selectids(condition);
		attributetable.remove(idlist);
	} catch (...) {
		debug(LOG_ERR, DEBUG_LOG, 0, "error while removing attributes");
		_database->rollback();
		return;
	}
	_database->commit();
}

/**
 * \brief Write metadata
 */
void	ImageDatabaseDirectory::writeMetadata(long imageid, ImagePtr image) {
	// work on the attribute table
	ImageAttributeTable	attributetable(_database);

	// add standard attributes
	ImageAttributeRecord	arecord(0, imageid);
	arecord.name = "SIMPLE";
	arecord.value = "T";
	arecord.comment = "file does conform to FITS standard";
	attributetable.add(arecord);

	arecord.name = "BITPIX";
	arecord.value = stringprintf("%d", image->bitsPerPixel());
	arecord.comment = "number of bits per data pixel";
	attributetable.add(arecord);

	arecord.name = "NAXIS";
	arecord.value = "3";
	arecord.comment = "number of data axes";
	attributetable.add(arecord);

	arecord.name = "NAXIS1";
	arecord.value = stringprintf("%d", image->size().width());
	arecord.comment = "length of data axis 1";
	attributetable.add(arecord);

	arecord.name = "NAXIS2";
	arecord.value = stringprintf("%d", image->size().height());
	arecord.comment = "length of data axis 2";
	attributetable.add(arecord);

	arecord.name = "NAXIS3";
	arecord.value = "1";
	arecord.comment = "length of data axis 3";
	attributetable.add(arecord);

	// add all attributes
	ImageMetadata::const_iterator	m;
	for (m = image->begin(); m != image->end(); m++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add attr %s",
			m->first.c_str());
		ImageAttributeRecord	attrs(0, imageid, *m);
		attributetable.add(attrs);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "image %d meta data added",
		imageid);
}

/**
 * \brief add an image to the directory and the database
 */
std::string	ImageDatabaseDirectory::save(ImagePtr image) {
	// first we add the image to the 
	std::string	filename;
	long	filesize;
	try {
		filename = ImageDirectory::save(image);
		filesize = fileSize(filename);
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not save the image: %s",
			x.what());
		throw x;
	}

	// if the database is not set, just ignore it, fall back to the 
	// behaviour of the ImageDirectory
	if (!_database) {
		debug(LOG_WARNING, DEBUG_LOG, 0, "warning: no database");
		return filename;
	}

	// now we have to add the image to the database to, if we run into
	// a problem here, we have to remove it in the exception handler
	_database->begin();
	try {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "add image %s to database",
			filename.c_str());
		// add record for the image
		ImageTable	imagetable(_database);
		ImageInfoRecord	record(0, filename, filesize, image);
		long	imageid = imagetable.add(record);

		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"image id: %d, %d metadata records", imageid,
			image->nMetadata());

		writeMetadata(imageid, image);
		
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "could not add image to db: %s",
			x.what());
		try {
			ImageDirectory::remove(filename);
		} catch (...) {
		}
		_database->rollback();
		throw x;
	}
	_database->commit();
	return filename;
}

/**
 * \brief Update the image in the database too
 */
void	ImageDatabaseDirectory::write(ImagePtr image,
		const std::string& filename) {
	// do the update to the file
	ImageDirectory::write(image, filename);

	// bracket the database changes in begin/commit block
	_database->begin();
	try {
		// get the image id
		ImageTable	imagetable(_database);
		std::string	condition = stringprintf("filename = '%s'",
					filename.c_str());
		long	imageid = imagetable.id(condition);
		ImageInfoRecord	record = imagetable.byid(imageid);
		record.filesize = fileSize(filename);
		imagetable.update(imageid, record);

		// handle the attributes
		ImageAttributeTable	attributetable(_database);
		condition = astro::stringprintf("image = %ld", imageid);
		attributetable.remove(condition);
		writeMetadata(imageid, image);
	} catch (const std::exception& x) {
		std::string	msg = stringprintf("unexpected exception while "
			"updateing image database for %s: %s",
			filename.c_str(), x.what());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		_database->rollback();
		throw x;
	}

	// commit the database changes
	_database->commit();
}

} // namespace image
} // namespace astro
