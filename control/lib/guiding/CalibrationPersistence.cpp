/*
 * CalibrationPersistence.cpp -- implementation of the calibration table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "CalibrationPersistence.h"

using namespace astro::persistence;

namespace astro {
namespace guiding {

//////////////////////////////////////////////////////////////////////
// Calibration constructor
//////////////////////////////////////////////////////////////////////
PersistentCalibration::PersistentCalibration() {
	time(&when);
	for (int i = 0; i < 6; i++) { a[i] = 0.; }
	focallength = 0;
	quality = 0;
	det = 0;
	complete = 0;
	masPerPixel = 0;
	controltype = 0;
}

PersistentCalibration::PersistentCalibration(const BasicCalibration& other) {
	// initialize fields that don't come from the basic calibration
	time(&when);

	// data from the basic calibration
	for (int i = 0; i < 6; i++) {
		a[i] = other.a[i];
	}
	complete = other.complete() ? 1 : 0;
	switch (other.calibrationtype()) {
	case GP:
		controltype = 0;
		break;
	case AO:
		controltype = 1;
		break;
	}
	quality = other.quality();
	det = other.det();

	// only available in the guidercalibration
	focallength = 0;
	masPerPixel = 0;
}

PersistentCalibration::PersistentCalibration(const GuiderCalibration& other) {
	// initialize fields that don't come from the basic calibration
	time(&when);

	// data from the basic calibration
	for (int i = 0; i < 6; i++) {
		a[i] = other.a[i];
	}
	complete = other.complete() ? 1 : 0;
	switch (other.calibrationtype()) {
	case GP:
		controltype = 0;
		break;
	case AO:
		controltype = 1;
		break;
	}
	quality = other.quality();
	det = other.det();

	// only available in the guidercalibration
	focallength = other.focallength;
	masPerPixel = other.masPerPixel;
}

PersistentCalibration&	PersistentCalibration::operator=(
	const BasicCalibration& other) {
	quality = other.quality();
	det = other.det();
	for (int i = 0; i < 6; i++) {
		a[i] = other.a[i];
	}
	return *this;
}

PersistentCalibration&	PersistentCalibration::operator=(
	const GuiderCalibration& other) {
	quality = other.quality();
	det = other.det();
	for (int i = 0; i < 6; i++) {
		a[i] = other.a[i];
	}
	focallength = other.focallength;
	masPerPixel = other.masPerPixel;
	return *this;
}

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
	"    name varchar(32) not null,\n"
	"    instrument varchar(32) not null,\n"
	"    ccd varchar(256) not null,\n"
	"    controldevice varchar(256) not null,\n"
	"    whenstarted datettime not null,\n"
	"    a0 double not null default 0,\n"
	"    a1 double not null default 0,\n"
	"    a2 double not null default 0,\n"
	"    a3 double not null default 0,\n"
	"    a4 double not null default 0,\n"
	"    a5 double not null default 0,\n"
	"    quality double not null default 0,\n"
	"    det double not null default 0,\n"
	"    complete int not null default 0,\n"
	"    focallength double not null default 0,\n"
	"    masperpixel double not null default 1,\n"
	"    controltype int not null default 0,\n"
	"    primary key(id)\n"
	")\n"
	);
}

CalibrationRecord	CalibrationTableAdapter::row_to_object(int objectid,
				const Row& row) {
	Persistent<PersistentCalibration>	result(objectid);
	result.name = row["name"]->stringValue();
	result.instrument = row["instrument"]->stringValue();
	result.ccd = row["ccd"]->stringValue();
	result.controldevice = row["controldevice"]->stringValue();
	result.when = row["whenstarted"]->timeValue();
	result.a[0] = row["a0"]->doubleValue();
	result.a[1] = row["a1"]->doubleValue();
	result.a[2] = row["a2"]->doubleValue();
	result.a[3] = row["a3"]->doubleValue();
	result.a[4] = row["a4"]->doubleValue();
	result.a[5] = row["a5"]->doubleValue();
	result.det = row["det"]->doubleValue();
	result.quality = row["quality"]->doubleValue();
	result.complete = row["complete"]->intValue();
	result.focallength = row["focallength"]->doubleValue();
	result.masPerPixel = row["masperpixel"]->doubleValue();
	result.controltype = row["controltype"]->intValue();
	return result;
}

UpdateSpec	CalibrationTableAdapter::object_to_updatespec(const CalibrationRecord& calibration) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("name", factory.get(calibration.name)));
	spec.insert(Field("instrument", factory.get(calibration.instrument)));
	spec.insert(Field("ccd", factory.get(calibration.ccd)));
	spec.insert(Field("controldevice", factory.get(calibration.controldevice)));
	spec.insert(Field("whenstarted", factory.getTime(calibration.when)));
	spec.insert(Field("a0", factory.get(calibration.a[0])));
	spec.insert(Field("a1", factory.get(calibration.a[1])));
	spec.insert(Field("a2", factory.get(calibration.a[2])));
	spec.insert(Field("a3", factory.get(calibration.a[3])));
	spec.insert(Field("a4", factory.get(calibration.a[4])));
	spec.insert(Field("a5", factory.get(calibration.a[5])));
	spec.insert(Field("quality", factory.get(calibration.quality)));
	spec.insert(Field("det", factory.get(calibration.det)));
	spec.insert(Field("complete", factory.get(calibration.complete)));
	spec.insert(Field("focallength", factory.get(calibration.focallength)));
	spec.insert(Field("masperpixel", factory.get(calibration.masPerPixel)));
	spec.insert(Field("controltype", factory.get(calibration.controltype)));
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
		"instrument = '%s' and ccd = '%s' and controldevice = '%s' "
		"order by whenstarted",
		guiderdescriptor.instrument().c_str(),
		guiderdescriptor.ccd().c_str(),
		guiderdescriptor.guiderport().c_str());
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
