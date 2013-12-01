/*
 * TaskTable.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <TaskTable.h>

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
	"create table ("
	"    id integer not null,"
	"    camera varchar(256) not null, "
	"    ccdid integer not null default 0, "
	"    temperature float not null default -1, "
	"    filterwheel varchar(256) not null default '', "
	"    position integer not null default 0, "
	"    originx integer not null default 0, "
	"    originy integer not null default 0, "
	"    width integer not null default 0, "
	"    height integer not null default 0, "
	"    exposuretime float not null default 1, "
	"    gain float not null, "
	"    limit float not null, "
	"    binx integer not null default 1, "
	"    biny integer not null default 1, "
	"    shutteropen integer not null default 1, "
	"    primary key(id) "
	")");
}

TaskQueueEntry	TaskTableAdapter::row_to_object(int objectid, const Row& row) {
	Task	task;
	task.camera(row["camera"]->stringValue());
	task.ccdid(row["ccdid"]->intValue());
	task.ccdtemperature(row["temperature"]->doubleValue());
	task.filterwheel(row["filterwheel"]->stringValue());
	task.filterposition(row["position"]->intValue());
	ImagePoint	origin(row["originx"]->intValue(),
				row["originy"]->intValue());
	ImageSize	size(row["width"]->intValue(),
				row["height"]->intValue());

	Exposure	exposure;
	exposure.frame.setOrigin(origin);
	exposure.frame.setSize(size);
	exposure.exposuretime = row["exposuretime"]->doubleValue();
	exposure.gain = row["gain"]->doubleValue();
	exposure.limit = row["limit"]->doubleValue();
	exposure.shutter = (row["shutteropen"]->intValue())
				? camera::SHUTTER_OPEN : camera::SHUTTER_CLOSED;

	Binning	mode(row["binx"]->intValue(), row["biny"]->intValue());
	exposure.mode = mode;
	task.exposure(exposure);
	TaskQueueEntry	entry(objectid, task);
	return entry;
}

UpdateSpec TaskTableAdapter::object_to_updatespec(const TaskQueueEntry& entry) {
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
	spec.insert(Field("limit", factory.get((double)exposure.limit)));
	spec.insert(Field("binx", factory.get((int)exposure.mode.getX())));
	spec.insert(Field("biny", factory.get((int)exposure.mode.getY())));
	spec.insert(Field("shutteropen",
		factory.get((exposure.shutter == SHUTTER_OPEN) ? 1 : 0)));
	return spec;
}

} // namespace task
} // namespace astro
