/*
 * ImageRepoTables.cpp -- image server table adapter implementations
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ImageRepoTables.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <includes.h>

namespace astro {
namespace project {

//////////////////////////////////////////////////////////////////////
// ImageInfo implementation (if necessary)
//////////////////////////////////////////////////////////////////////
ImageInfo::ImageInfo() {
	project = "unknown";
	created = time(NULL);
	pixeltype = 8;
	depth = 1;
	exposuretime = 0;
	temperature = 0;
	purpose = "light";
	bayer = "    ";
	observation = "1970-01-01T00:00:00.000";
	xbin = 1;
	ybin = 1;
}

bool	ImageInfo::operator==(const ImageInfo& other) const {
	if (filename != other.filename) { return false; }
	if (project != other.project) { return false; }
	if (created != other.created) { return false; }
	if (camera != other.camera) { return false; }
	if (width != other.width) { return false; }
	if (height != other.height) { return false; }
	if (xbin != other.xbin) { return false; }
	if (ybin != other.ybin) { return false; }
	if (depth != other.depth) { return false; }
	if (pixeltype != other.pixeltype) { return false; }
	if (exposuretime != other.exposuretime) { return false; }
	if (temperature != other.temperature) { return false; }
	if (purpose != other.purpose) { return false; }
	if (bayer != other.bayer) { return false; }
	if (observation != other.observation) { return false; }
	if (uuid != other.uuid) { return false; }
	return true;
}

//////////////////////////////////////////////////////////////////////
// Image table adapter implementation
//////////////////////////////////////////////////////////////////////

std::string	ImageTableAdapter::tablename() {
	return std::string("images");
}

std::string	ImageTableAdapter::createstatement() {
	return std::string(
		"create table images (\n"
		"    id integer not null,\n"
		"    filename varchar(1024) not null,\n"
		"    project varchar(128) not null,\n"
		"    created datetime not null,\n"
		"    camera varchar(128) not null,\n"
		"    width int not null,\n"
		"    height int not null,\n"
		"    xbin int not null,\n"
		"    ybin int not null,\n"
		"    depth int not null default 1,\n"
		"    pixeltype int not null default 16,\n"
		"    exposuretime float not null default 1,\n"
		"    temperature float not null default 0,\n"
		"    purpose char(5) not null default 'light',\n"
		"    bayer char(4) not null default '    ',\n"
		"    observation varchar(25) not null,\n"
		"    uuid varchar(36) not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index images_x1 on images(filename);\n"
		"create unique index images_x2 on images(uuid);\n"
	);
}

ImageRecord	ImageTableAdapter::row_to_object(int objectid,
				const Row& row) {
	ImageRecord	record(objectid);
	record.filename = row["filename"]->stringValue();
	record.project = row["project"]->stringValue();
	record.created = row["created"]->timeValue();
	record.camera = row["camera"]->stringValue();
	record.width = row["width"]->intValue();
	record.height = row["height"]->intValue();
	record.xbin = row["xbin"]->intValue();
	record.ybin = row["ybin"]->intValue();
	record.depth = row["depth"]->intValue();
	record.pixeltype = row["pixeltype"]->intValue();
	record.exposuretime = row["exposuretime"]->doubleValue();
	record.temperature = row["temperature"]->doubleValue();
	record.purpose = row["purpose"]->stringValue();
	record.bayer = row["bayer"]->stringValue();
	record.observation = row["observation"]->stringValue();
	record.uuid = row["uuid"]->stringValue();
	return record;
}

UpdateSpec	ImageTableAdapter::object_to_updatespec(const ImageRecord& imagerec) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("filename", factory.get(imagerec.filename)));
	spec.insert(Field("project", factory.get(imagerec.project)));
	spec.insert(Field("created", factory.getTime(imagerec.created)));
	spec.insert(Field("camera", factory.get(imagerec.camera)));
	spec.insert(Field("width", factory.get(imagerec.width)));
	spec.insert(Field("height", factory.get(imagerec.height)));
	spec.insert(Field("depth", factory.get(imagerec.depth)));
	spec.insert(Field("xbin", factory.get(imagerec.xbin)));
	spec.insert(Field("ybin", factory.get(imagerec.ybin)));
	spec.insert(Field("pixeltype", factory.get(imagerec.pixeltype)));
	spec.insert(Field("exposuretime", factory.get(imagerec.exposuretime)));
	spec.insert(Field("temperature", factory.get(imagerec.temperature)));
	spec.insert(Field("purpose", factory.get(imagerec.purpose)));
	spec.insert(Field("bayer", factory.get(imagerec.bayer)));
	spec.insert(Field("observation", factory.get(imagerec.observation)));
	spec.insert(Field("uuid", factory.get(imagerec.uuid)));
	return spec;
}

//////////////////////////////////////////////////////////////////////
// ImageTable implementation
//////////////////////////////////////////////////////////////////////
long	ImageTable::id(const std::string& filename) {
	std::string	condition = stringprintf("filename = '%s'",
				filename.c_str());
	std::list<object_type>	objects = select(condition);
	if (objects.size() == 0) {
		std::string	msg = stringprintf("no image with filename %s",
			filename.c_str());
		throw std::runtime_error(msg);
	}
	return objects.begin()->id();
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
		"    imageid integer not null references images(id) "
			"on delete cascade on update cascade,\n"
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
