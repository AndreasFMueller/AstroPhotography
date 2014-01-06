/*
 * Calibration.cpp -- implementation of the calibration table
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <Calibration.h>

using namespace astro::persistence;

namespace astro {
namespace guiding {

std::string	CalibrationTableAdapter::tablename() {
	return std::string("calibration");
}

std::string	CalibrationTableAdapter::createstatement() {
	return std::string(
	"create table calibration (\n"
	"    id integer not null,\n"
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

Calibration	CalibrationTableAdapter::row_to_object(int objectid, const Row& row) {
	Calibration	result;
	result.id(objectid);
	result.when = row["whenstarted"]->timeValue();
	result.a[0] = row["a0"]->doubleValue();
	result.a[1] = row["a1"]->doubleValue();
	result.a[2] = row["a2"]->doubleValue();
	result.a[3] = row["a3"]->doubleValue();
	result.a[4] = row["a4"]->doubleValue();
	result.a[5] = row["a5"]->doubleValue();
	return result;
}

UpdateSpec	CalibrationTableAdapter::object_to_updatespec(const Calibration& calibration) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("whenstarted", factory.getTime(calibration.when)));
	spec.insert(Field("a0", factory.get(calibration.a[0])));
	spec.insert(Field("a1", factory.get(calibration.a[1])));
	spec.insert(Field("a2", factory.get(calibration.a[2])));
	spec.insert(Field("a3", factory.get(calibration.a[3])));
	spec.insert(Field("a4", factory.get(calibration.a[4])));
	spec.insert(Field("a5", factory.get(calibration.a[5])));
	return spec;
}

std::string	CalibrationPointTableAdapter::tablename() {
	return std::string("calibrationpoint");
}

std::string	CalibrationPointTableAdapter::createstatement() {
	return std::string(
	"create table calibrationpoint (\n"
	"    id int not null,\n"
	"    calibration int not null,\n"
	"    t double not null default 0,\n"
	"    ra double not null default 0,\n"
	"    dec double not null default 0,\n"
	"    x double not null default 0,\n"
	"    y double not null default 0,\n"
	"    primary key(id)\n"
	")\n"
	);
}

CalibrationPoint	CalibrationPointTableAdapter::row_to_object(int objectid, const Row& row) {
	CalibrationPoint	point(objectid);
	point.calibration = row["calibration"]->intValue();
	point.t = row["t"]->doubleValue();
	point.ra = row["ra"]->doubleValue();
	point.dec = row["dec"]->doubleValue();
	point.x = row["x"]->doubleValue();
	point.y = row["y"]->doubleValue();
	return point;
}

UpdateSpec	CalibrationPointTableAdapter::object_to_updatespec(const CalibrationPoint& point) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("calibration", factory.get(point.calibration)));
	spec.insert(Field("t", factory.get(point.t)));
	spec.insert(Field("ra", factory.get(point.ra)));
	spec.insert(Field("dec", factory.get(point.dec)));
	spec.insert(Field("x", factory.get(point.x)));
	spec.insert(Field("y", factory.get(point.y)));
	return spec;
}

} // namespace guiding
} // namespace astro
