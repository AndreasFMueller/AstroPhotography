/*
 * Stellarium.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil 
 */
#ifndef _Stellarium_h
#define _Stellarium_h

#include <AstroCatalog.h>
#include <map>

namespace astro {
namespace catalog {

/**
 * \brief DeepSky catalog based on Stellarium database
 */
class Stellarium : public std::map<int, DeepSkyObject> {
	std::map<std::string, int>	_ngc_map;
	std::map<std::string, int>	_ic_map;
	std::map<std::string, int>	_m_map;
	std::map<std::string, int>	_c_map;
	std::map<std::string, int>	_b_map;
	std::map<std::string, int>	_sh2_map;
	std::map<std::string, int>	_vdb_map;
	std::map<std::string, int>	_rcw_map;
	std::map<std::string, int>	_ldn_map;
	std::map<std::string, int>	_lbn_map;
	std::map<std::string, int>	_cr_map;
	std::map<std::string, int>	_mel_map;
	std::map<std::string, int>	_pgc_map;
	std::map<std::string, int>	_ugc_map;
	std::map<std::string, int>	_ced_map;
	std::map<std::string, int>	_arp_map;
	std::map<std::string, int>	_vv_map;
	std::map<std::string, int>	_pk_map;
	std::map<std::string, int>	_pn_map;
	std::map<std::string, int>	_snr_map;
	std::map<std::string, int>	_aco_map;
	std::map<std::string, int>	_hcg_map;
	std::map<std::string, int>	_abbell_map;
	std::map<std::string, int>	_eso_map;
	void	parse(const std::string& filename);
public:
	Stellarium();
	Stellarium(const std::string& filename);
	DeepSkyObject	find(const std::string& name) const;
	DeepSkyObjectSetPtr	find(const SkyWindow& window) const;
	std::set<std::string>	findLike(const std::string& name) const;
};

} // namespace catalog
} // namespace astro

#endif /* _Stellarium_h */
