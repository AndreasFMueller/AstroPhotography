/*
 * ImagePersistence.h -- table of images and image attributes
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswi
 */
#ifndef _ImagePersistence_h
#define _ImagePersistence_h

#include <AstroPersistence.h>
#include <AstroImage.h>

namespace astro {
namespace image {

/**
 * \brief Class for common image data
 *
 * This class contains the mandatory attributes that are contained in
 * all images
 */
class ImageInfo {
public:
	ImageInfo() { }
	ImageInfo(const std::string& filename, long filesize, ImagePtr image);
	std::string	filename;
	time_t	created;
	int	filesize;
	int	width;
	int	height;
};

class ImageInfoRecord : public persistence::Persistent<ImageInfo> {
public:
	ImageInfoRecord(int id) : persistence::Persistent<ImageInfo>(id) { }
	ImageInfoRecord(int id, const std::string& filename, long filesize,
		ImagePtr image) : persistence::Persistent<ImageInfo>(id,
			ImageInfo(filename, filesize, image)) {
	}
};

/**
 * \brief Table adapter for Image info records
 */
class ImageTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static ImageInfoRecord
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const ImageInfoRecord& imageinfo);
};

class ImageTable : public astro::persistence::Table<ImageInfoRecord, ImageTableAdapter> {
public:
	ImageTable(astro::persistence::Database& database)
		: astro::persistence::Table<ImageInfoRecord,
			ImageTableAdapter>(database) {
	}
};

/** 
 * \brief Class for 
 */
class ImageAttribute {
public:
	std::string	name;
	std::string	value;
	std::string	comment;
	ImageAttribute() { }
	ImageAttribute(const std::pair<std::string, Metavalue>& v)
		: name(v.first), value(v.second.getValue()),
		  comment(v.second.getComment()) {
	}
};

class ImageAttributeRecord : public persistence::PersistentRef<ImageAttribute> {
public:
	ImageAttributeRecord(int id, int ref) 
		: persistence::PersistentRef<ImageAttribute>(id, ref) { }
	ImageAttributeRecord(int id, int ref,
		const std::pair<std::string, Metavalue>& v)
		: persistence::PersistentRef<ImageAttribute>(id, ref,
			ImageAttribute(v)) {
	}
};

/**
 * \brief Table adapter for image attributes
 */
class ImageAttributeAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static ImageAttributeRecord
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const ImageAttributeRecord& imageattribute);
};

class ImageAttributeTable : public astro::persistence::Table<ImageAttributeRecord, ImageAttributeAdapter> {
public:
	ImageAttributeTable(astro::persistence::Database& database)
		: astro::persistence::Table<ImageAttributeRecord,
			ImageAttributeAdapter>(database) {
	}
};

} // namespace image
} // namespace astro

#endif /* _ImagePersistence_h */
