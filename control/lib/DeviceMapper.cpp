/*
 * DeviceMapper.cpp --  Device mapper class implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroConfig.h>
#include <DeviceMapTable.h>
#include <AstroFormat.h>
#include <AstroDebug.h>

namespace astro {
namespace config {

/**
 * \brief DeviceMapper backend class that does the actual work
 *
 * This class is used so that the DeviceMapTable needs not to be exposed
 */
class DeviceMapperBackend : public DeviceMapper {
	Database	database;
	DeviceMapTable	devicemap;
	DeviceMap	select(const std::string& condition);
	int	selectid(const std::string& condition);
public:
	DeviceMapperBackend(Database _database)
		: database(_database), devicemap(_database) { }
	int	id(const std::string& name);
	int	id(const DeviceName& name, const std::string& servername);
	virtual DeviceMap	find(const std::string& name);
	virtual DeviceMap	find(const DeviceName& devicename,
					const std::string& servername);
	virtual void    add(const DeviceMap& devicemap);
private:
	void	update(int id, const DeviceMap& d);
public:
	virtual void    update(const std::string& name, const DeviceMap& d);
	virtual void    update(const DeviceName& devicename, 
				const std::string& servername,
				const DeviceMap& d);
	virtual void	remove(const std::string& name);
	virtual void	remove(const DeviceName& devicename,
				const std::string& servername);
	virtual DeviceMapperPtr	devicemapper();
};

/**
 * \brief Select a single entry based on a condition
 */
DeviceMap	DeviceMapperBackend::select(const std::string& condition) {
	std::list<DeviceMapRecord>	records = devicemap.select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no devicemap entry for %s",
			condition.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	DeviceMapRecord	record = *records.begin();
	DeviceMap	result(DeviceName(record.devicename));
	result.name(record.name);
	result.servername(record.servername);
	result.description(record.description);
	return result;
}

/**
 * \brief Get the id of an entry based on a condition
 */
int	DeviceMapperBackend::selectid(const std::string& condition) {
	std::list<DeviceMapRecord>	records = devicemap.select(condition);
	if (0 == records.size()) {
		std::string	msg = stringprintf("no devicemap entry for %s",
			condition.c_str());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	return records.begin()->id();
}

/**
 * \brief Get the id of an entry based on the  name
 */
int	DeviceMapperBackend::id(const std::string& name) {
	std::string	condition = stringprintf("name = '%s'",
		database->escape(name).c_str());
	return selectid(condition);
}

/**
 * \brief Get the id of a map entry based on device name and server
 */
int	DeviceMapperBackend::id(const DeviceName& devicename,
			const std::string& servername) {
	std::string	condition = stringprintf(
		"devicename = '%s' and servername = '%s'", 
		database->escape(devicename.toString()).c_str(),
		database->escape(servername).c_str());
	return selectid(condition);
}

/**
 * \brief retrieve an entry based on the name of the entry
 */
DeviceMap	DeviceMapperBackend::find(const std::string& name) {
	std::string	condition = stringprintf("name = '%s'",
		database->escape(name).c_str());
	return select(condition);
}

/**
 * \brief Retrieve a name based on the device name and the server name
 */
DeviceMap	DeviceMapperBackend::find(const DeviceName& devicename,
			const std::string& servername) {
	std::string	condition = stringprintf(
		"devicename = '%s' and servername = '%s'", 
		database->escape(devicename.toString()).c_str(),
		database->escape(servername).c_str());
	return select(condition);
}

/**
 * \brief Add a map entry
 */
void	DeviceMapperBackend::add(const DeviceMap& d) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "adding entry for '%s'",
		d.name().c_str());
	DeviceMapRecord	record;
	record.name = d.name();
	record.devicename = d.devicename().toString();
	record.servername = d.servername();
	record.description = d.description();
	devicemap.add(record);
}

void	DeviceMapperBackend::update(int id, const DeviceMap& d) {
	DeviceMapRecord	record(id);
	record.name = d.name();
	record.devicename = d.devicename().toString();
	record.servername = d.servername();
	record.description = d.description();
	devicemap.update(id, record);
}

/**
 * \brief update an entry identified by name
 */
void	DeviceMapperBackend::update(const std::string& name,
		const DeviceMap& dm) {
	update(id(name), dm);
}

/**
 * \brief update an entry identified by device name and server
 */
void	DeviceMapperBackend::update(const DeviceName& devicename,
		const std::string& servername, const DeviceMap& dm) {
	update(id(devicename, servername), dm);
}

/**
 * \brief Remove a map entry based on the name
 */
void	DeviceMapperBackend::remove(const std::string& name) {
	devicemap.remove(id(name));
}

/**
 * \brief Remove a map entry based on the devicename and servername
 */
void	DeviceMapperBackend::remove(const DeviceName& devicename,
		const std::string& servername) {
	devicemap.remove(id(devicename, servername));
}

/**
 * \brief 
 */
DeviceMapperPtr	DeviceMapperBackend::devicemapper() {
	return DeviceMapper::get(database);
}

//////////////////////////////////////////////////////////////////////
// DeviceMapper
//////////////////////////////////////////////////////////////////////
/**
 * \brief Build a new DeviceMapper
 */
DeviceMapperPtr	DeviceMapper::get(Database database) {
	return DeviceMapperPtr(new DeviceMapperBackend(database));
}

} // namespace config
} // namespace astro
