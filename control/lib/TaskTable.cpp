/*
 * TaskTable.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskTable.h>
#include <AstroDebug.h>

using namespace astro::persistence;
using namespace astro::image;
using namespace astro::camera;

namespace astro {
namespace task {

std::string	TaskTableAdapter::tablename() {
	return std::string("taskqueue");
}

std::string	TaskTableAdapter::createstatement() {
	return std::string(
	"create table taskqueue (\n"
	"    id integer not null,\n"
	"    camera varchar(256) not null,\n"
	"    ccdid integer not null default 0,\n"
	"    temperature float not null default -1,\n"
	"    filterwheel varchar(256) not null default '',\n"
	"    position integer not null default 0,\n"
	"    originx integer not null default 0,\n"
	"    originy integer not null default 0,\n"
	"    width integer not null default 0,\n"
	"    height integer not null default 0,\n"
	"    exposuretime float not null default 1,\n"
	"    gain float not null,\n"
	"    vlimit float not null,\n"
	"    binx integer not null default 1,\n"
	"    biny integer not null default 1,\n"
	"    shutteropen integer not null default 1,\n"
	"    state integer not null default 0,\n"
	"    lastchange integer not null default 0,\n"
	"    cause varchar(256) not null default '',\n"
	"    filename varchar(256) not null default '',\n"
	"    primary key(id)\n"
	")");
}

TaskQueueEntry	TaskTableAdapter::row_to_object(int objectid, const Row& row) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert object %d", objectid);
	TaskParameters	parameters;
	parameters.camera(row["camera"]->stringValue());
	parameters.ccdid(row["ccdid"]->intValue());
	parameters.ccdtemperature(row["temperature"]->doubleValue());
	parameters.filterwheel(row["filterwheel"]->stringValue());
	parameters.filterposition(row["position"]->intValue());
	ImagePoint	origin(row["originx"]->intValue(),
				row["originy"]->intValue());
	ImageSize	size(row["width"]->intValue(),
				row["height"]->intValue());

	Exposure	exposure;
	exposure.frame.setOrigin(origin);
	exposure.frame.setSize(size);
	exposure.exposuretime = row["exposuretime"]->doubleValue();
	exposure.gain = row["gain"]->doubleValue();
	exposure.limit = row["vlimit"]->doubleValue();
	exposure.shutter = (row["shutteropen"]->intValue())
				? camera::SHUTTER_OPEN : camera::SHUTTER_CLOSED;

	Binning	mode(row["binx"]->intValue(), row["biny"]->intValue());
	exposure.mode = mode;
	parameters.exposure(exposure);

	TaskQueueEntry	entry(objectid, parameters);
	entry.state((TaskQueueEntry::taskstate)row["state"]->intValue());
	entry.lastchange(row["lastchange"]->intValue());
	entry.cause(row["cause"]->stringValue());
	entry.filename(row["filename"]->stringValue());
	debug(LOG_DEBUG, DEBUG_LOG, 0, "conversion complete");

	return entry;
}

UpdateSpec TaskTableAdapter::object_to_updatespec(const TaskQueueEntry& entry) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert entry %d", entry.id());
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("camera", factory.get(entry.camera())));
	spec.insert(Field("ccdid", factory.get((int)entry.ccdid())));
	spec.insert(Field("temperature", factory.get(entry.ccdtemperature())));
	spec.insert(Field("filterwheel", factory.get(entry.filterwheel())));
	spec.insert(Field("position", factory.get((int)entry.filterposition())));
	Exposure	exposure = entry.exposure();
	ImageRectangle	frame = exposure.frame;
	spec.insert(Field("originx", factory.get((int)frame.origin().x())));
	spec.insert(Field("originy", factory.get((int)frame.origin().y())));
	spec.insert(Field("width", factory.get((int)frame.size().width())));
	spec.insert(Field("height", factory.get((int)frame.size().height())));

	spec.insert(Field("exposuretime",
		factory.get((double)exposure.exposuretime)));
	spec.insert(Field("gain", factory.get((double)exposure.gain)));
	spec.insert(Field("vlimit", factory.get((double)exposure.limit)));
	spec.insert(Field("binx", factory.get((int)exposure.mode.getX())));
	spec.insert(Field("biny", factory.get((int)exposure.mode.getY())));
	spec.insert(Field("shutteropen",
		factory.get((exposure.shutter == SHUTTER_OPEN) ? 1 : 0)));

	spec.insert(Field("state", factory.get((int)entry.state())));
	spec.insert(Field("lastchange", factory.get((int)entry.lastchange())));
	spec.insert(Field("cause", factory.get(entry.cause())));
	spec.insert(Field("filename", factory.get(entry.filename())));
	return spec;
}

} // namespace task
} // namespace astro
