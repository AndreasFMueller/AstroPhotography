/*
 * InstrumentTables.cpp -- implementation of instrument tables
 *
 * (c) 2014 Prof Dr Andraes Mueller, Hochschule Rapperswil
 */
#include <InstrumentTables.h>
#include <AstroFormat.h>

namespace astro {
namespace config {

//////////////////////////////////////////////////////////////////////
// Instrument table adapter
//////////////////////////////////////////////////////////////////////

std::string	InstrumentTableAdapter::tablename() {
	return std::string("instruments");
}

std::string	InstrumentTableAdapter::createstatement() {
	return std::string(
		"create table instruments (\n"
		"    id integer not null,\n"
		"    name integer not null,\n"
		"    primary key(id)\n"
		");\n"
		"create unique index instruments_idx1 on instruments(name);\n"
	);
}

InstrumentRecord	InstrumentTableAdapter::row_to_object(int objectid, const Row& row) {
	InstrumentRecord	record(objectid);
	record.name = row["name"]->stringValue();
	return record;
}

UpdateSpec	InstrumentTableAdapter::object_to_updatespec(const InstrumentRecord& instrument) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("name", factory.get(instrument.name)));
	return spec;
}

long	InstrumentTable::id(const std::string& name) {
	std::string	condition = stringprintf("name = '%s'",
				_database->escape(name).c_str());
	std::list<long>	ids = selectids(condition);
	if (ids.size() == 1) {
		return *ids.begin();
	}
	throw std::runtime_error("name not found");
}

//////////////////////////////////////////////////////////////////////
// InstrumentComponent table adapter
//////////////////////////////////////////////////////////////////////

std::string	InstrumentComponentTableAdapter::type(DeviceName::device_type t) {
	switch (t) {
	case DeviceName::AdaptiveOptics:
		return std::string("adaptiveoptics");
	case DeviceName::Camera:
		return std::string("camera");
	case DeviceName::Ccd:
		return std::string("ccd");
	case DeviceName::Cooler:
		return std::string("cooler");
	case DeviceName::Filterwheel:
		return std::string("filterwheel");
	case DeviceName::Focuser:
		return std::string("focuser");
	case DeviceName::Guiderport:
		return std::string("guiderport");
	case DeviceName::Module:
		return std::string("module");
	case DeviceName::Mount:
		return std::string("mount");
	}
	throw std::runtime_error("unknown type");
}

DeviceName::device_type	InstrumentComponentTableAdapter::type(const std::string& t) {
	if (t == "adaptiveoptics") {
		return DeviceName::AdaptiveOptics;
	}
	if (t == "camera") {
		return DeviceName::Camera;
	}
	if (t == "ccd") {
		return DeviceName::Ccd;
	}
	if (t == "cooler") {
		return DeviceName::Cooler;
	}
	if (t == "filterwheel") {
		return DeviceName::Filterwheel;
	}
	if (t == "focuser") {
		return DeviceName::Focuser;
	}
	if (t == "guiderport") {
		return DeviceName::Guiderport;
	}
	if (t == "module") {
		return DeviceName::Module;
	}
	if (t == "mount") {
		return DeviceName::Mount;
	}
	throw std::runtime_error("unknown type");
}

std::string	InstrumentComponentTableAdapter::component_type(InstrumentComponent::component_t c) {
	switch (c) {
	case InstrumentComponent::direct:
		return std::string("direct");
	case InstrumentComponent::mapped:
		return std::string("mapped");
	case InstrumentComponent::derived:
		return std::string("derived");
	}
	throw std::runtime_error("unknown component type");
}

InstrumentComponent::component_t	InstrumentComponentTableAdapter::component_type(const std::string& c) {
	if (c == "direct") {
		return InstrumentComponent::direct;
	}
	if (c == "mapped") {
		return InstrumentComponent::mapped;
	}
	if (c == "derived") {
		return InstrumentComponent::derived;
	}
	throw std::runtime_error("unknown component type");
}

std::string	InstrumentComponentTableAdapter::tablename() {
	return std::string("components");
}

std::string	InstrumentComponentTableAdapter::createstatement() {
	return std::string(
		"create table components (\n"
		"    id integer not null,\n"
		"    instrument integer not null references instruments(id) "
			"on delete cascade on update cascade,\n"
		"    type varchar(16) not null,\n"
		"    componenttype varchar(16) not null,\n"
		"    device varchar(128) not null,\n"
		"    unit int not null,\n"
		"    servername varchar(128) not null default '',\n"
		"    primary key(id)\n"
		");\n"
		"create unique index components_idx1 "
			"on components(id, instrument);\n"
		"create unique index components_idx2 "
			"on components(id, type);\n"
	);
}

InstrumentComponentRecord	InstrumentComponentTableAdapter::row_to_object(int objectid,
	const Row& row) {
        int     instrumentid = row["instrument"]->intValue();
	InstrumentComponentRecord	record(objectid, instrumentid);
	record.type = row["type"]->stringValue();
	record.componenttype = row["componenttype"]->stringValue();
	record.devicename = row["device"]->stringValue();
	record.unit = row["unit"]->intValue();
	record.servername = row["servername"]->stringValue();
	return record;
}

UpdateSpec	InstrumentComponentTableAdapter::object_to_updatespec(const InstrumentComponentRecord& component) {
	UpdateSpec	spec;
	FieldValueFactory	factory;
	spec.insert(Field("instrument", factory.get(component.ref())));
	spec.insert(Field("type", factory.get(component.type)));
	spec.insert(Field("componenttype",
				factory.get(component.componenttype)));
	spec.insert(Field("device", factory.get(component.devicename)));
	spec.insert(Field("unit", factory.get(component.unit)));
	spec.insert(Field("servername", factory.get(component.servername)));
	return spec;
}

} // namespace config
} // namespace astro
