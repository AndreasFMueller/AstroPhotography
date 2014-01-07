/*
 * Tracking.cpp -- persistence of tracking data
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Tracking.h>

using namespace astro::persistence;

namespace astro {
namespace guiding {

std::string	GuidingRunTableAdapter::tablename() {
	return std::string("guidingrun");
}

std::string	GuidingRunTableAdapter::createstatement() {
	return std::string(
	"create table guidingrun (\n"
	"    id integer not null,\n"
	"    camera varchar(256) not null,\n"
	"    ccdid integer not null default 0,\n"
	"    guiderport varchar(256) not null,\n"
	"    whenstarted datetime not null,\n"
	"    primary key(id)\n"
	")\n"
	);
}

GuidingRun	GuidingRunTableAdapter::row_to_object(int objectid,
			const Row& row) {
	GuidingRun	result;
	result.whenstarted = row["whenstarted"]->timeValue();
	result.camera = row["camera"]->stringValue();
	result.ccdid = row["ccdid"]->intValue();
	result.guiderport = row["guiderport"]->stringValue();
	return result;
}

UpdateSpec	GuidingRunTableAdapter::object_to_updatespec(const GuidingRun& guidingrun) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("camera", factory.get(guidingrun.camera)));
	spec.insert(Field("ccdid", factory.get(guidingrun.ccdid)));
	spec.insert(Field("guiderport", factory.get(guidingrun.guiderport)));
	spec.insert(Field("whenstarted", factory.getTime(guidingrun.whenstarted)));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "update spec has %d entries", spec.size());
	return spec;
}

std::string	TrackingTableAdapter::tablename() {
	return std::string("tracking");
}

std::string	TrackingTableAdapter::createstatement() {
	return std::string(
	"create table tracking (\n"
	"    id integer not null,\n"
	"    guidingrun integer not null,\n"
	"    trackingtime double not null,\n"
	"    xoffset double not null,\n"
	"    yoffset double not null,\n"
	"    racorrection double not null,\n"
	"    deccorrection double not null,\n"
	"    primary key(id)\n"
	")\n"
	);
}

Tracking	TrackingTableAdapter::row_to_object(int objectid,
			const Row& row) {
	Tracking	tracking(objectid);
	tracking.when = row["trackingtime"]->doubleValue();
	tracking.guidingrun = row["guidingrun"]->intValue();
	tracking.xoffset = row["xoffset"]->doubleValue();
	tracking.yoffset = row["yoffset"]->doubleValue();
	tracking.racorrection = row["racorrection"]->doubleValue();
	tracking.deccorrection = row["deccorrection"]->doubleValue();
	return tracking;
}

UpdateSpec	TrackingTableAdapter::object_to_updatespec(const Tracking& tracking) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("trackingtime", factory.get(tracking.when)));
	spec.insert(Field("guidingrun", factory.get(tracking.guidingrun)));
	spec.insert(Field("xoffset", factory.get(tracking.xoffset)));
	spec.insert(Field("yoffset", factory.get(tracking.yoffset)));
	spec.insert(Field("racorrection", factory.get(tracking.racorrection)));
	spec.insert(Field("deccorrection", factory.get(tracking.deccorrection)));
	return spec;
}

} // namespace guiding
} // namespace astro
