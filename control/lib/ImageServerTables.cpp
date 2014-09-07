/*
 * ImageServerTables.cpp -- image server table adapter implementations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageServerTables.h>

namespace astro {
namespace project {

//////////////////////////////////////////////////////////////////////
// ImageServerInfo implementation (if necessary)
//////////////////////////////////////////////////////////////////////
bool	ImageServerInfo::operator==(const ImageServerInfo& other) const {
	if (filename != other.filename) { return false; }
	if (project != other.project) { return false; }
	if (created != other.created) { return false; }
	if (width != other.width) { return false; }
	if (height != other.height) { return false; }
	if (depth != other.depth) { return false; }
	if (pixeltype != other.pixeltype) { return false; }
	if (exposuretime != other.exposuretime) { return false; }
	if (temperature != other.temperature) { return false; }
	if (category != other.category) { return false; }
	if (bayer != other.bayer) { return false; }
	if (observation != other.observation) { return false; }
	return true;
}

//////////////////////////////////////////////////////////////////////
// ImageServer table adapter implementation
//////////////////////////////////////////////////////////////////////

std::string	ImageServerTableAdapter::tablename() {
	return std::string("imageserver");
}

std::string	ImageServerTableAdapter::createstatement() {
	return std::string(
		"create table imageserver (\n"
		"    id integer not null,\n"
		"    filename varchar(1024) not null,\n"
		"    project varchar(128) not null,\n"
		"    created datetime not null,\n"
		"    width int not null,\n"
		"    height int not null,\n"
		"    depth int not null default 1,\n"
		"    pixeltype int not null default 16,\n"
		"    exposuretime float not null default 1,\n"
		"    temperature float not null default 0,\n"
		"    category char(5) not null default 'light',\n"
		"    bayer char(4) not null default '    ',\n"
		"    observation varchar(25) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index imageserver_x1 on imageserver(filename);\n"
	);
}

ImageServerRecord	ImageServerTableAdapter::row_to_object(int objectid,
				const Row& row) {
	ImageServerRecord	record(objectid);
	record.filename = row["filename"]->stringValue();
	record.project = row["project"]->stringValue();
	record.created = row["created"]->timeValue();
	record.width = row["width"]->intValue();
	record.height = row["height"]->intValue();
	record.depth = row["depth"]->intValue();
	record.pixeltype = row["pixeltype"]->intValue();
	record.exposuretime = row["exposuretime"]->doubleValue();
	record.temperature = row["temperature"]->doubleValue();
	record.category = row["category"]->stringValue();
	record.bayer = row["bayer"]->stringValue();
	record.observation = row["observation"]->stringValue();
	return record;
}

UpdateSpec	ImageServerTableAdapter::object_to_updatespec(const ImageServerRecord& imagerec) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("filename", factory.get(imagerec.filename)));
	spec.insert(Field("project", factory.get(imagerec.project)));
	spec.insert(Field("created", factory.getTime(imagerec.created)));
	spec.insert(Field("width", factory.get(imagerec.width)));
	spec.insert(Field("height", factory.get(imagerec.height)));
	spec.insert(Field("depth", factory.get(imagerec.depth)));
	spec.insert(Field("pixeltype", factory.get(imagerec.pixeltype)));
	spec.insert(Field("exposuretime", factory.get(imagerec.exposuretime)));
	spec.insert(Field("temperature", factory.get(imagerec.temperature)));
	spec.insert(Field("category", factory.get(imagerec.category)));
	spec.insert(Field("bayer", factory.get(imagerec.bayer)));
	spec.insert(Field("observation", factory.get(imagerec.observation)));
	return spec;
}

//////////////////////////////////////////////////////////////////////
// MetadataInfo implementation (if necessary)
//////////////////////////////////////////////////////////////////////
bool	MetadataInfo::operator==(const MetadataInfo& other) const {
	if (seqno != other.seqno) { return false; }
	if (key != other.key) { return false; }
	if (value != other.value) { return false; }
	if (comment != other.comment) { return false; }
	return true;
}

//////////////////////////////////////////////////////////////////////
// Metadata table adapter
//////////////////////////////////////////////////////////////////////
std::string	MetadataTableAdapter::tablename() {
	return std::string("metadata");
}

std::string	MetadataTableAdapter::createstatement() {
	return std::string(
		"create table metadata (\n"
		"    id integer not null,\n"
		"    imageid integer not null references imageserver(id),\n"
		"    seqno integer not null,\n"
		"    key char(8) not null,\n"
		"    value varchar(72),\n"
		"    comment varchar(72) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index metadata_x1 on metadata(imageid, seqno);\n"
	);
}

MetadataRecord	MetadataTableAdapter::row_to_object(int objectid,
			const Row& row) {
	int	ref = row["imageid"]->intValue();
	MetadataRecord	record(objectid, ref);
	record.seqno = row["seqno"]->intValue();
	record.key = row["key"]->stringValue();
	record.value = row["value"]->stringValue();
	record.comment = row["comment"]->stringValue();
	return record;
}

UpdateSpec	MetadataTableAdapter::object_to_updatespec(const MetadataRecord&metarec) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("imageid", factory.get(metarec.ref())));
	spec.insert(Field("seqno", factory.get(metarec.seqno)));
	spec.insert(Field("key", factory.get(metarec.key)));
	spec.insert(Field("value", factory.get(metarec.value)));
	spec.insert(Field("comment", factory.get(metarec.comment)));
	return spec;
}

} // namespace project
} // namespace astro
