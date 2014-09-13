/*
 * Calibration.cpp -- implementation of the calibration table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <CalibrationPersistence.h>

using namespace astro::persistence;

namespace astro {
namespace guiding {

//////////////////////////////////////////////////////////////////////
// CalibrationTableAdapter implementation
//////////////////////////////////////////////////////////////////////
std::string	CalibrationTableAdapter::tablename() {
	return std::string("calibration");
}

std::string	CalibrationTableAdapter::createstatement() {
	return std::string(
	"create table calibration (\n"
	"    id integer not null,\n"
	"    camera varchar(128) not null,\n"
	"    ccdid integer not null,\n"
	"    guiderport integer not null,\n"
	"    whenstarted datettime not null,\n"
	"    a0 double not null default 0,\n"
	"    a1 double not null default 0,\n"
	"    a2 double not null default 0,\n"
	"    a3 double not null default 0,\n"
	"    a4 double not null default 0,\n"
	"    a5 double not null default 0,\n"
	"    primary key(id)\n"
	")\n"
	);
}

CalibrationRecord	CalibrationTableAdapter::row_to_object(int objectid, const Row& row) {
	Persistent<Calibration>	result(objectid);
	result.camera = row["camera"]->stringValue();
	result.ccdid = row["ccdid"]->intValue();
	result.guiderport = row["guiderport"]->stringValue();
	result.when = row["whenstarted"]->timeValue();
	result.a[0] = row["a0"]->doubleValue();
	result.a[1] = row["a1"]->doubleValue();
	result.a[2] = row["a2"]->doubleValue();
	result.a[3] = row["a3"]->doubleValue();
	result.a[4] = row["a4"]->doubleValue();
	result.a[5] = row["a5"]->doubleValue();
	return result;
}

UpdateSpec	CalibrationTableAdapter::object_to_updatespec(const CalibrationRecord& calibration) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("camera", factory.get(calibration.camera)));
	spec.insert(Field("ccdid", factory.get(calibration.ccdid)));
	spec.insert(Field("guiderport", factory.get(calibration.guiderport)));
	spec.insert(Field("whenstarted", factory.getTime(calibration.when)));
	spec.insert(Field("a0", factory.get(calibration.a[0])));
	spec.insert(Field("a1", factory.get(calibration.a[1])));
	spec.insert(Field("a2", factory.get(calibration.a[2])));
	spec.insert(Field("a3", factory.get(calibration.a[3])));
	spec.insert(Field("a4", factory.get(calibration.a[4])));
	spec.insert(Field("a5", factory.get(calibration.a[5])));
	return spec;
}

//////////////////////////////////////////////////////////////////////
// CalibrationTable implementation
//////////////////////////////////////////////////////////////////////
CalibrationTable::CalibrationTable(Database& database)
	: Table<CalibrationRecord, CalibrationTableAdapter>(database) {
}

/**
 * \brief Retrieve calibration ids for a selected guider
 */
std::list<long>	CalibrationTable::selectids(
	const GuiderDescriptor& guiderdescriptor) {
	std::string	condition = stringprintf(
		"camera = '%s' and ccdid = %d and guiderport = '%s' "
		"order by whenstarted",
		guiderdescriptor.cameraname().c_str(),
		guiderdescriptor.ccdid(),
		guiderdescriptor.guiderportname().c_str());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "condition for calibrations: %s",
		condition.c_str());	
	return selectids(condition);
}

//////////////////////////////////////////////////////////////////////
// CalibrationPointTableAdapter implementation
//////////////////////////////////////////////////////////////////////
std::string	CalibrationPointTableAdapter::tablename() {
	return std::string("calibrationpoint");
}

std::string	CalibrationPointTableAdapter::createstatement() {
	return std::string(
	"create table calibrationpoint (\n"
	"    id int not null,\n"
	"    calibration int not null references calibration(id) "
		"on delete cascade on update cascade,\n"
	"    t double not null default 0,\n"
	"    ra double not null default 0,\n"
	"    dec double not null default 0,\n"
	"    x double not null default 0,\n"
	"    y double not null default 0,\n"
	"    primary key(id)\n"
	")\n"
	);
}

CalibrationPointRecord	CalibrationPointTableAdapter::row_to_object(int objectid, const Row& row) {
	double	t = row["t"]->doubleValue();

	double	ra = row["ra"]->doubleValue();
	double	dec = row["dec"]->doubleValue();
	Point	offset(ra, dec);

	double	x = row["x"]->doubleValue();
	double	y = row["y"]->doubleValue();
	Point	star(x, y);

	CalibrationPoint	calpoint(t, offset, star);

	int	ref = row["calibration"]->intValue();

	CalibrationPointRecord	point(objectid, ref, calpoint);
	return point;
}

UpdateSpec	CalibrationPointTableAdapter::object_to_updatespec(const CalibrationPointRecord& point) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("calibration", factory.get(point.ref())));
	spec.insert(Field("t", factory.get(point.t)));
	spec.insert(Field("ra", factory.get(point.offset.x())));
	spec.insert(Field("dec", factory.get(point.offset.y())));
	spec.insert(Field("x", factory.get(point.star.x())));
	spec.insert(Field("y", factory.get(point.star.y())));
	return spec;
}

} // namespace guiding
} // namespace astro
