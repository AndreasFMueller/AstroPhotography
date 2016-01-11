/*
 * Tracking.cpp -- persistence of tracking data
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TrackingPersistence.h>

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
	"    instrument varchar(32) not null,\n"
	"    ccd varchar(256) not null default 0,\n"
	"    guiderport varchar(256) not null,\n"
	"    whenstarted datetime not null,\n"
	"    calibration integer not null,\n"
	"    primary key(id)\n"
	")\n"
	);
}

GuidingRunRecord	GuidingRunTableAdapter::row_to_object(int objectid,
			const Row& row) {
	Persistent<GuidingRun>	result(objectid);
	result.whenstarted = row["whenstarted"]->timeValue();
	result.instrument = row["instrument"]->stringValue();
	result.ccd = row["ccd"]->stringValue();
	result.guiderport = row["guiderport"]->stringValue();
	result.calibrationid = row["calibration"]->intValue();
	return result;
}

UpdateSpec	GuidingRunTableAdapter::object_to_updatespec(const GuidingRunRecord& guidingrun) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("instrument", factory.get(guidingrun.instrument)));
	spec.insert(Field("ccd", factory.get(guidingrun.ccd)));
	spec.insert(Field("guiderport", factory.get(guidingrun.guiderport)));
	spec.insert(Field("whenstarted", factory.getTime(guidingrun.whenstarted)));
	spec.insert(Field("calibration", factory.get(guidingrun.calibrationid)));
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
	"    guidingrun integer not null references guidingrun(id) "
	"	on delete cascade on update cascade,\n"
	"    trackingtime double not null,\n"
	"    xoffset double not null,\n"
	"    yoffset double not null,\n"
	"    racorrection double not null,\n"
	"    deccorrection double not null,\n"
	"    primary key(id)\n"
	")\n"
	);
}

TrackingPointRecord	TrackingTableAdapter::row_to_object(int objectid,
			const Row& row) {
	double	when = row["trackingtime"]->doubleValue();
	Point	offset(row["xoffset"]->doubleValue(),
			row["yoffset"]->doubleValue());
	Point	correction(row["racorrection"]->doubleValue(),
			row["deccorrection"]->doubleValue());
	TrackingPoint	ti(when, offset, correction);
	
	TrackingPointRecord	tracking(objectid,
		row["guidingrun"]->intValue(), ti);
	return tracking;
}

UpdateSpec	TrackingTableAdapter::object_to_updatespec(const TrackingPointRecord& tracking) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("trackingtime", factory.get(tracking.t)));
	spec.insert(Field("guidingrun", factory.get(tracking.ref())));
	spec.insert(Field("xoffset", factory.get(tracking.trackingoffset.x())));
	spec.insert(Field("yoffset", factory.get(tracking.trackingoffset.y())));
	spec.insert(Field("racorrection", factory.get(tracking.correction.x())));
	spec.insert(Field("deccorrection", factory.get(tracking.correction.y())));
	return spec;
}

} // namespace guiding
} // namespace astro
