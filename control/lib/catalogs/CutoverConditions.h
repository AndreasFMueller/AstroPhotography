/*
 * CutoverConditions.h -- conditions used as cutovers between the various
 *                        catalogs
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#ifndef _CutoverConditions_h
#define _CutoverConditions_h

#include <AstroCatalog.h>

namespace astro {
namespace catalog {

class CutoverCondition {
	char	_catalog;
	float	_cutover_mag;
	float	_limit_mag;
	uint64_t	_count;
	uint64_t	_duplicates;
	uint64_t	_toofaint;
	uint64_t	_toobright;
	bool	_lowcut;
public:
	uint64_t	count() const { return _count; }
	uint64_t	duplicates() const { return _duplicates; }
	uint64_t	toofaint() const { return _toofaint; }
	uint64_t	toobright() const { return _toobright; }
	bool	lowcut() const { return _lowcut; }
	void	lowcut(bool l) { _lowcut = l; }
	std::string	toString() const;
	static const float	unlimited;
	CutoverCondition(char catalog, float cutover_mag, float limit_mag);
	bool	operator()(const Star& star);
};
typedef std::shared_ptr<CutoverCondition>	CutoverConditionPtr;

class BSCCondition : public CutoverCondition {
public:
	static const float	complete_mag;
	BSCCondition(const float limit_mag);
	BSCCondition();
};

class HipparcosCondition : public CutoverCondition {
public:
	static const float	complete_mag;
	HipparcosCondition(const float limit_mag, float bsc_mag);
	HipparcosCondition(const float limit_mag);
	HipparcosCondition();
};

class Tycho2Condition : public CutoverCondition {
public:
	static const float	complete_mag;
	Tycho2Condition(float limit_mag, float hipparcos_mag);
	Tycho2Condition(float limit_mag);
	Tycho2Condition();
};

class Ucac4Condition : public CutoverCondition {
public:
	static const float	complete_mag;
	Ucac4Condition(float limit_mag, float tycho2_mag);
	Ucac4Condition(float limit_mag);
	Ucac4Condition();
};

} // namespace catalog
} // namespace astro

#endif /* _CutoverConditions_h */
