/*
 * ImagePersistence.cpp -- table for images and image attributes
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImagePersistence.h>

using namespace astro::persistence;

namespace astro {
namespace image {

//////////////////////////////////////////////////////////////////////
// ImageAdapter implemenation
//////////////////////////////////////////////////////////////////////
std::string	ImageTableAdapter::tablename() {
	return std::string("images");
}

std::string	ImageTableAdapter::createstatement() {
	return std::string(
		"create table images (\n"
		"    id integer not null,\n"
		"    filename varchar(1024) not null,\n"
		"    created datetime not null,\n"
		"    filesize int not null,\n"
		"    width int not null,\n"
		"    height int not null,\n"
		"    primary key(id)\n"
		")\n"
	);
}

ImageInfoRecord	ImageTableAdapter::row_to_object(int objectid,
			const Row& row) {
	ImageInfoRecord	record(objectid);
	record.filename = row["filename"]->stringValue();
	record.created = row["created"]->timeValue();
	record.filesize = row["filesize"]->intValue();
	record.width = row["width"]->intValue();
	record.height = row["height"]->intValue();
	return record;
}

UpdateSpec	ImageTableAdapter::object_to_updatespec(const ImageInfoRecord& imageinfo) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("filename", factory.get(imageinfo.filename)));
	spec.insert(Field("filesize", factory.get(imageinfo.filesize)));
	spec.insert(Field("created", factory.getTime(imageinfo.created)));
	spec.insert(Field("width", factory.get(imageinfo.width)));
	spec.insert(Field("height", factory.get(imageinfo.height)));
	return spec;
}

ImageInfo::ImageInfo(const std::string& _filename, long _filesize,
	ImagePtr image) : filename(_filename), filesize(_filesize) {
	created = time(NULL);
	width = image->size().width();
	height = image->size().height();
}

//////////////////////////////////////////////////////////////////////
// ImageAttribute Adapter implemenation
//////////////////////////////////////////////////////////////////////
std::string	ImageAttributeAdapter::tablename() {
	return std::string("imageattributes");
}

std::string	ImageAttributeAdapter::createstatement() {
	return std::string(
		"create table imageattributes (\n"
		"    id integer not null,\n"
		"    image integer not null references images(id),\n"
		"    attribute char(8) not null,\n"
		"    value char(8) not null,\n"
		"    comment varchar(128) not null,\n"
		"    primary key (id)\n"
		")\n"
	);
}

ImageAttributeRecord	ImageAttributeAdapter::row_to_object(int objectid,
				const Row& row) {
	int	ref = row["image"]->intValue();
	ImageAttributeRecord	record(objectid, ref);
	record.name = row["attribute"]->stringValue();
	record.value = row["value"]->stringValue();
	record.comment = row["comment"]->stringValue();
	return record;
}

UpdateSpec	ImageAttributeAdapter::object_to_updatespec(
	const ImageAttributeRecord& imageattribute) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("image", factory.get(imageattribute.ref())));
	spec.insert(Field("attribute", factory.get(imageattribute.name)));
	spec.insert(Field("value", factory.get(imageattribute.value)));
	spec.insert(Field("comment", factory.get(imageattribute.comment)));
	return spec;
}

} // namespace image 
} // namespace astro
