/*
 * Tracking.cpp -- persistence of tracking data
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "TrackingPersistence.h"

using namespace astro::persistence;

namespace astro {
namespace guiding {

std::string	TrackTableAdapter::tablename() {
	return std::string("track");
}

std::string	TrackTableAdapter::createstatement() {
	return std::string(
	"create table track (\n"
	"    id integer not null,\n"
	"    instrument varchar(32) not null,\n"
	"    ccd varchar(256) not null default 0,\n"
	"    guideport varchar(256) not null,\n"
	"    adaptiveoptics varchar(256) not null,\n"
	"    whenstarted datetime not null,\n"
	"    guideportcalid integer not null,\n"
	"    adaptiveopticscalid integer not null,\n"
	"    primary key(id)\n"
	")\n"
	);
}

TrackRecord	TrackTableAdapter::row_to_object(int objectid,
			const Row& row) {
	Persistent<Track>	result(objectid);
	result.whenstarted = row["whenstarted"]->timeValue();
	result.instrument = row["instrument"]->stringValue();
	result.ccd = row["ccd"]->stringValue();
	result.guideport = row["guideport"]->stringValue();
	result.adaptiveoptics = row["adaptiveoptics"]->stringValue();
	result.guideportcalid = row["guideportcalid"]->intValue();
	result.adaptiveopticscalid = row["adaptiveopticscalid"]->intValue();
	return result;
}

UpdateSpec	TrackTableAdapter::object_to_updatespec(const TrackRecord& track) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("instrument", factory.get(track.instrument)));
	spec.insert(Field("ccd", factory.get(track.ccd)));
	spec.insert(Field("guideport", factory.get(track.guideport)));
	spec.insert(Field("adaptiveoptics", factory.get(track.adaptiveoptics)));
	spec.insert(Field("whenstarted", factory.getTime(track.whenstarted)));
	spec.insert(Field("guideportcalid", factory.get(track.guideportcalid)));
	spec.insert(Field("adaptiveopticscalid", factory.get(track.adaptiveopticscalid)));
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
	"    track integer not null references track(id) "
	"	on delete cascade on update cascade,\n"
	"    trackingtime double not null,\n"
	"    xoffset double not null,\n"
	"    yoffset double not null,\n"
	"    racorrection double not null,\n"
	"    deccorrection double not null,\n"
	"    controltype int not null default 0,\n"
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
	switch (row["controltype"]->intValue()) {
	case 0: ti.type = GP;
		break;
	case 1:	ti.type = AO;
		break;
	}
	TrackingPointRecord	tracking(objectid,
		row["track"]->intValue(), ti);
	return tracking;
}

UpdateSpec	TrackingTableAdapter::object_to_updatespec(const TrackingPointRecord& tracking) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("trackingtime", factory.get(tracking.t)));
	spec.insert(Field("track", factory.get(tracking.ref())));
	spec.insert(Field("xoffset", factory.get(tracking.trackingoffset.x())));
	spec.insert(Field("yoffset", factory.get(tracking.trackingoffset.y())));
	spec.insert(Field("racorrection", factory.get(tracking.correction.x())));
	spec.insert(Field("deccorrection", factory.get(tracking.correction.y())));
	switch (tracking.type) {
	case GP:
		spec.insert(Field("controltype", factory.get((int)0)));
		break;
	case AO:
		spec.insert(Field("controltype", factory.get((int)1)));
		break;
	}
	return spec;
}

} // namespace guiding
} // namespace astro
