/*
 * ImageRepoTables.h -- Tables for the image repository
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageRepoTables_h
#define _ImageRepoTables_h

#include <AstroPersistence.h>

using namespace astro::persistence;

namespace astro {
namespace project {

/**
 * \brief The data contained in the image server table
 */
class ImageInfo {
public:
	std::string	filename;
	std::string	project;
	time_t	created;
	std::string	camera;
	int	width;
	int	height;
	int	depth;
	int	xbin;
	int	ybin;
	int	pixeltype;
	double	exposuretime;
	double	temperature;
	std::string	category;
	std::string	bayer;
	std::string	observation;
	std::string	uuid;
	bool	operator==(const ImageInfo& other) const;
	ImageInfo();
};

/**
 * \brief Wrapper around the image info that adds the object id
 */
class ImageRecord : public Persistent<ImageInfo> {
public:
	ImageRecord(int id = -1) : Persistent<ImageInfo>(id) { }
};

/**
 * \brief Adapter for the images table
 */
class ImageTableAdapter {
public:
static std::string      tablename();
static std::string      createstatement();
static ImageRecord row_to_object(int objectid, const Row& row);
static UpdateSpec object_to_updatespec(const ImageRecord& imageinfo);
};

/**
 * \brief The table for image info
 */
class ImageTable : public Table<ImageRecord, ImageTableAdapter> {
public:
	ImageTable(Database database)
		: Table<ImageRecord, ImageTableAdapter>(database) {
	}
	long	id(const std::string& filename);
};

/**
 * \brief The data contained in the metadata table
 */
class MetadataInfo {
public:
	int	seqno;
	std::string	key;
	std::string	value;
	std::string	comment;
	bool	operator==(const MetadataInfo& other) const;
};

/**
 * \brief Wrapper for the metadata information
 */
class MetadataRecord : public PersistentRef<MetadataInfo> {
public:
	MetadataRecord(int id, int ref) : PersistentRef<MetadataInfo>(id, ref) { }
	bool	operator<(const MetadataRecord& other) const {
		if (id() == other.id()) {
			return seqno < other.seqno;
		}
		return id() < other.id();
	}
};

/**
 * \brief Adapter for the metadata table
 */
class MetadataTableAdapter {
public:
static std::string      tablename();
static std::string      createstatement();
static MetadataRecord
        row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
        object_to_updatespec(const MetadataRecord& imageinfo);
};

/**
 * \brief Metadata table
 */
class MetadataTable : public Table<MetadataRecord, MetadataTableAdapter> {
public:
	MetadataTable(Database& database)
		: Table<MetadataRecord, MetadataTableAdapter>(database) { }
};

} // namespace project
} // namespace astro

#endif /* _ImageRepoTables_h */
