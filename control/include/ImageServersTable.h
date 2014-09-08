/*
 * ImageServersTable.h -- a table of image servers
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroPersistence.h>
#include <AstroProject.h>

using namespace astro::persistence;

namespace astro {
namespace project {

/**
 * \brief A class describing an image server
 */
class ImageServerInfo {
public:
	std::string	servername;
	std::string	database;
	std::string	directory;
	bool	operator==(const ImageServerInfo& other) const;
};

/**
 * \brief Wrapper around the image server class, adds the object id
 */
class ImageServerRecord : public Persistent<ImageServerInfo> {
public:
	ImageServerRecord(int id = -1) : Persistent<ImageServerInfo>(id) { }
	bool	operator==(const ImageServerRecord& other) const;
};

/**
 * \Adapter for the image server table
 */
class ImageServerTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static ImageServerRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec object_to_updatespec(const ImageServerRecord& imageserver);
};

/**
 * \brief The table for image server info
 */
class ImageServerTable : public Table<ImageServerRecord, ImageServerTableAdapter> {
public:
	ImageServerTable(Database& database)
		: Table<ImageServerRecord, ImageServerTableAdapter>(database) {
	}
	ImageServer	get(const std::string& name);
};

} // namespace astro
} // namespace project
