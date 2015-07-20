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

} // namespace image
} // namespace astro
