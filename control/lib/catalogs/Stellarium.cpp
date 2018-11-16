/*
 * Stellarium.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "Stellarium.h"
#include <AstroDebug.h>
#include <AstroFormat.h>
#include <fstream>

namespace astro {
namespace catalog {

#define cross_number(index, prefix, catalog) 				\
	if (components[index].size() > 0) {				\
		std::string	key = stringprintf("%s%d", prefix,	\
			std::stoi(components[index]));			\
		catalog.insert(std::make_pair(key, object.number));	\
	}

#define cross_name(index, catalog)					\
	if (components[index].size() > 0) {				\
		std::string	key = components[index];		\
		catalog.insert(std::make_pair(key, object.number));	\
	}

void	Stellarium::parse(const std::string& filename) {
	// open the file
	std::ifstream	in(filename.c_str());
	if (!in.good()) {
		std::string     msg = stringprintf("cannot get %s",
			filename.c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	// start reading into the buffer
	char	buffer[2048];
	while (!in.eof()) {
		// read a new line
		in.getline(buffer, sizeof(buffer));

		// skip comment lines
		if (buffer[0] == '#')
			continue;

		// split a record
		std::string	s(buffer);
		std::vector<std::string>	components;
		split(s, "\t", components);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "found %d components",
			components.size());

		// create a new object
		DeepSkyObject	object;
		object.number = std::stoi(components[0]);
		object.ra() = astro::Angle(stod(components[1]),
					astro::Angle::Degrees);
		object.dec() = astro::Angle(stod(components[2]),
					astro::Angle::Degrees);
		object.mag(std::stod(components[4]));
		object.size.a1() = Angle(std::stod(components[8]) / 60,
			astro::Angle::Degrees);
		object.size.a2() = Angle(std::stod(components[9]) / 60,
			astro::Angle::Degrees);
		object.azimuth = Angle(std::stod(components[10]),
			astro::Angle::Degrees);

		// insert the object
		insert(std::make_pair(object.number, object));

		// add the cross maps
		cross_number(17, "NGC", _ngc_map)
		cross_number(18, "IC", _ic_map)
		cross_number(19, "M", _m_map)
		cross_number(20, "C", _c_map)
		cross_number(21, "B", _b_map)
		cross_number(22, "Sh2", _sh2_map)
		cross_number(23, "VdB", _vdb_map)
		cross_number(24, "RCW", _rcw_map)
		cross_number(25, "LDN", _ldn_map)
		cross_number(26, "LBN", _lbn_map)
		cross_number(27, "Cr", _cr_map)
		cross_number(28, "Mel", _mel_map)
		cross_number(29, "PGC", _pgc_map)
		cross_number(30, "UGC", _ugc_map)

		cross_name(31, _ced_map)
		cross_number(32, "Arp", _arp_map)
		cross_number(33, "VV", _vv_map)
		cross_name(34, _pk_map)
		cross_name(35, _pn_map)
		cross_name(36, _snr_map)
		cross_name(37, _aco_map)
		cross_name(38, _hcg_map)
		cross_name(39, _abbell_map)
		cross_name(40, _eso_map)
	}

	// catalog parsed
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s contains %d objets",
		filename.c_str(), size());
}

Stellarium::Stellarium() {
	std::string	pathbase(DATAROOTDIR "/starcatalogs/stellarium");
	std::string	filename = pathbase + std::string("/catalog.txt");
	parse(filename);
}

Stellarium::Stellarium(const std::string& filename) {
	parse(filename);
}

#define	check_catalog(catalog, name)					\
{									\
	auto i = catalog.find(name);					\
	if (i != catalog.end()) {					\
		int	number = i->second;				\
		return std::map<int, DeepSkyObject>::find(number)->second;\
	}								\
}

DeepSkyObject	Stellarium::find(const std::string& name) const {
	check_catalog(_ngc_map, name)
	check_catalog(_ic_map, name)
	check_catalog(_m_map, name)
	check_catalog(_c_map, name)
	check_catalog(_b_map, name)
	check_catalog(_sh2_map, name)
	check_catalog(_vdb_map, name)
	check_catalog(_rcw_map, name)
	check_catalog(_ldn_map, name)
	check_catalog(_lbn_map, name)
	check_catalog(_cr_map, name)
	check_catalog(_mel_map, name)
	check_catalog(_pgc_map, name)
	check_catalog(_ugc_map, name)
	check_catalog(_ced_map, name)
	check_catalog(_arp_map, name)
	check_catalog(_vv_map, name)
	check_catalog(_pk_map, name)
	check_catalog(_pn_map, name)
	check_catalog(_snr_map, name)
	check_catalog(_aco_map, name)
	check_catalog(_hcg_map, name)
	check_catalog(_abbell_map, name)
	check_catalog(_eso_map, name)
	std::string	msg = stringprintf("'%s' not found", name.c_str());
	debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
	throw std::runtime_error(msg);
}

DeepSkyCatalog::deepskyobjectsetptr	Stellarium::find(
						const SkyWindow& window) const {
	DeepSkyCatalog::deepskyobjectset	*result
		= new DeepSkyCatalog::deepskyobjectset();
	DeepSkyCatalog::deepskyobjectsetptr	resultptr(result);
	std::for_each(begin(), end(),
		[&](const std::pair<int, DeepSkyObject>& p) {
			if (window.contains(p.second.position(2000))) {
				result->insert(p.second);
			}
		}
	);
	return resultptr;
}

} // namespace catalog
} // namespace astro
