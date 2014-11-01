/*
 * ImageReposTable.h -- a table of image repositories
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageReposTable_h
#define _ImageReposTable_h

#include <AstroPersistence.h>
#include <AstroProject.h>

using namespace astro::persistence;

namespace astro {
namespace project {

/**
 * \brief Wrapper around the image server class, adds the object id
 */
class ImageRepoRecord : public Persistent<ImageRepoInfo> {
public:
	ImageRepoRecord(int id = -1) : Persistent<ImageRepoInfo>(id) { }
	bool	operator==(const ImageRepoRecord& other) const;
};

/**
 * \Adapter for the image server table
 */
class ImageRepoTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static ImageRepoRecord	row_to_object(int objectid, const Row& row);
static UpdateSpec object_to_updatespec(const ImageRepoRecord& imageserver);
};

/**
 * \brief The table for image server info
 */
class ImageRepoTable : public Table<ImageRepoRecord, ImageRepoTableAdapter> {
public:
	ImageRepoTable(Database& database)
		: Table<ImageRepoRecord, ImageRepoTableAdapter>(database) {
	}
	ImageRepo	get(const std::string& name);
	void	remove(const std::string& name);
};

} // namespace astro
} // namespace project

#endif /* _ImageReposTable_h */
