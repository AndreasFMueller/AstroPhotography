/*
 * ImageServerTables.h
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageServerTables_h
#define _ImageServerTables_h

#include <AstroPersistence.h>

using namespace astro::persistence;

namespace astro {
namespace project {

/**
 * \brief The data contained in the image server table
 */
class ImageServerInfo {
public:
	std::string	filename;
	std::string	project;
	time_t	created;
	int	width;
	int	height;
	int	depth;
	int	pixeltype;
	double	exposuretime;
	double	temperature;
	std::string	category;
	std::string	bayer;
	std::string	observation;
	bool	operator==(const ImageServerInfo& other) const;
};

/**
 * \brief Wrapper around the image info that adds the object id
 */
class ImageServerRecord : public Persistent<ImageServerInfo> {
public:
	ImageServerRecord(int id)
		: Persistent<ImageServerInfo>(id) { }
	ImageServerRecord()
		: Persistent<ImageServerInfo>(-1) { }
};

/**
 * \brief Adapter for the imageserver table
 */
class ImageServerTableAdapter {
public:
static std::string      tablename();
static std::string      createstatement();
static ImageServerRecord row_to_object(int objectid, const Row& row);
static UpdateSpec object_to_updatespec(const ImageServerRecord& imageinfo);
};

/**
 * \brief The talbe for Image server info
 */
class ImageServerTable : public Table<ImageServerRecord, ImageServerTableAdapter> {
public:
	ImageServerTable(Database database)
		: Table<ImageServerRecord, ImageServerTableAdapter>(database) {
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

#endif /* _ImageServerTables_h */
