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
	"    instrument varchar(32) not null,\n"
	"    cameraindex int not null default -1,\n"
	"    camera varchar(256) not null default '',\n"
	"    ccdindex int not null default -1,\n"
	"    ccd varchar(256) not null default '',\n"
	"    coolerindex int not null default -1,\n"
	"    cooler varchar(256) not null default '',\n"
	"    temperature float not null default -1,\n"
	"    filterwheelindex int not null default -1,\n"
	"    filterwheel varchar(256) not null default '',\n"
	"    filter varchar(32) not null default '',\n"
	"    mountindex int not null default -1,\n"
	"    mount varchar(256) not null default '',\n"
	"    focuserindex int not null default -1,\n"
	"    focuser varchar(256) not null default '',\n"
	"    originx integer not null default 0,\n"
	"    originy integer not null default 0,\n"
	"    width integer not null default 0,\n"
	"    height integer not null default 0,\n"
	"    exposuretime float not null default 1,\n"
	"    gain float not null default 1,\n"
	"    vlimit float not null,\n"
	"    binx integer not null default 1,\n"
	"    biny integer not null default 1,\n"
	"    shutteropen integer not null default 1,\n"
	"    purpose integer not null default 0,\n"
	"    state integer not null default 0,\n"
	"    lastchange integer not null default 0,\n"
	"    cause varchar(256) not null default '',\n"
	"    filename varchar(256) not null default '',\n"
	"    imagex integer not null default 0,\n"
	"    imagey integer not null default 0,\n"
	"    imagewidth integer not null default 0,\n"
	"    imageheight integer not null default 0,\n"
	"    project varchar(32) not null default '',\n"
	"    repodb varchar(1024) not null default '',\n"
	"    repository varchar(32) not null default '',\n"
	"    primary key(id)\n"
	")");
}

TaskQueueEntry	TaskTableAdapter::row_to_object(int objectid, const Row& row) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert object %d", objectid);
	TaskParameters	parameters;
	parameters.instrument(row["instrument"]->stringValue());
	parameters.cameraindex(row["cameraindex"]->intValue());
	parameters.ccdindex(row["ccdindex"]->intValue());
	parameters.coolerindex(row["coolerindex"]->intValue());
	parameters.ccdtemperature(row["temperature"]->doubleValue());
	parameters.filterwheelindex(row["filterwheelindex"]->intValue());
	parameters.filter(row["filter"]->stringValue());
	parameters.mountindex(row["mountindex"]->intValue());
	parameters.focuserindex(row["focuserindex"]->intValue());
	parameters.project(row["project"]->stringValue());
	parameters.repodb(row["repodb"]->stringValue());
	parameters.repository(row["repository"]->stringValue());
	ImagePoint	origin(row["originx"]->intValue(),
				row["originy"]->intValue());
	ImageSize	size(row["width"]->intValue(),
				row["height"]->intValue());

	Exposure	exposure;
	ImageRectangle	frame(origin, size);
	exposure.frame(frame);
	exposure.exposuretime(row["exposuretime"]->doubleValue());
	exposure.gain(row["gain"]->doubleValue());
	exposure.limit(row["vlimit"]->doubleValue());
	exposure.shutter((row["shutteropen"]->intValue())
				? camera::Shutter::OPEN
				: camera::Shutter::CLOSED);
	exposure.purpose((Exposure::purpose_t)row["purpose"]->intValue());

	Binning	mode(row["binx"]->intValue(), row["biny"]->intValue());
	exposure.mode(mode);
	parameters.exposure(exposure);

	TaskQueueEntry	entry(objectid, parameters);

	entry.camera(row["camera"]->stringValue());
	entry.ccd(row["ccd"]->stringValue());
	entry.cooler(row["cooler"]->stringValue());
	entry.filterwheel(row["filterwheel"]->stringValue());
	entry.mount(row["mount"]->stringValue());
	entry.focuser(row["focuser"]->stringValue());
	entry.state((TaskQueueEntry::taskstate)row["state"]->intValue());
	entry.lastchange(row["lastchange"]->intValue());
	entry.cause(row["cause"]->stringValue());
	entry.filename(row["filename"]->stringValue());
	entry.size(ImageSize(row["imagewidth"]->intValue(),
			row["imageheight"]->intValue()));
	entry.origin(ImagePoint(row["imagex"]->intValue(),
			row["imagey"]->intValue()));

	debug(LOG_DEBUG, DEBUG_LOG, 0, "conversion complete");

	return entry;
}

UpdateSpec TaskTableAdapter::object_to_updatespec(const TaskQueueEntry& entry) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "convert entry %d", entry.id());
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("instrument", factory.get(entry.instrument())));
	spec.insert(Field("camera", factory.get(entry.camera())));
	spec.insert(Field("cameraindex", factory.get(entry.cameraindex())));
	spec.insert(Field("ccd", factory.get(entry.ccd())));
	spec.insert(Field("ccdindex", factory.get(entry.ccdindex())));

	spec.insert(Field("cooler", factory.get(entry.cooler())));
	spec.insert(Field("coolerindex", factory.get(entry.coolerindex())));
	spec.insert(Field("temperature", factory.get(entry.ccdtemperature())));

	spec.insert(Field("filterwheel", factory.get(entry.filterwheel())));
	spec.insert(Field("filterwheelindex", factory.get(entry.filterwheelindex())));
	spec.insert(Field("filter", factory.get(entry.filter())));

	spec.insert(Field("mount", factory.get(entry.mount())));
	spec.insert(Field("mountindex", factory.get(entry.mountindex())));
	spec.insert(Field("focuser", factory.get(entry.focuser())));
	spec.insert(Field("focuserindex", factory.get(entry.focuserindex())));

	Exposure	exposure = entry.exposure();
	ImageRectangle	frame = exposure.frame();
	spec.insert(Field("originx", factory.get((int)frame.origin().x())));
	spec.insert(Field("originy", factory.get((int)frame.origin().y())));
	spec.insert(Field("width", factory.get((int)frame.size().width())));
	spec.insert(Field("height", factory.get((int)frame.size().height())));

	spec.insert(Field("exposuretime",
		factory.get((double)exposure.exposuretime())));
	spec.insert(Field("gain", factory.get((double)exposure.gain())));
	spec.insert(Field("vlimit", factory.get((double)exposure.limit())));
	spec.insert(Field("binx", factory.get((int)exposure.mode().x())));
	spec.insert(Field("biny", factory.get((int)exposure.mode().y())));
	spec.insert(Field("shutteropen",
		factory.get((exposure.shutter() == Shutter::OPEN) ? 1 : 0)));
	spec.insert(Field("purpose", factory.get((int)exposure.purpose())));

	spec.insert(Field("state", factory.get((int)entry.state())));
	spec.insert(Field("lastchange", factory.get((int)entry.lastchange())));
	spec.insert(Field("cause", factory.get(entry.cause())));
	spec.insert(Field("filename", factory.get(entry.filename())));
	spec.insert(Field("imagex", factory.get((int)entry.origin().x())));
	spec.insert(Field("imagey", factory.get((int)entry.origin().y())));
	spec.insert(Field("imagewidth", factory.get((int)entry.size().width())));
	spec.insert(Field("imageheight", factory.get((int)entry.size().height())));

	spec.insert(Field("project", factory.get(entry.project())));
	spec.insert(Field("repodb", factory.get(entry.repodb())));
	spec.insert(Field("repository", factory.get(entry.repository())));

	return spec;
}

} // namespace task
} // namespace astro
