/*
 * CutoverConditions.cpp -- Cutover condition implementation
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rappeswil
 */
#include "CutoverConditions.h"
#include <AstroFormat.h>

namespace astro {
namespace catalog {

//////////////////////////////////////////////////////////////////////
// CutoverCondition implementation
//////////////////////////////////////////////////////////////////////
const float CutoverCondition::unlimited = 100.;

CutoverCondition::CutoverCondition(char catalog, float cutover_mag,
	float limit_mag)
	: _catalog(catalog), _cutover_mag(cutover_mag), _limit_mag(limit_mag) {
	_duplicates = 0;
	_toofaint = 0;
	_toobright = 0;
}

bool	CutoverCondition::operator()(const Star& star) {
	_count++;
	// duplicates below the cutover magnitude are eliminated
	if (_catalog) {
		if (star.isDuplicate() && (_catalog == star.duplicateCatalog())
			&& star.mag() < _cutover_mag) {
			_duplicates++;
			return false;
		}
	}
	// to bright objects are rejected if that is enabled
	if (_lowcut) {
		if (star.mag() < _cutover_mag) {
			_toobright++;
			return false;
		}
	}
	// only keep stars below the limiting magnitude or the cutover
	// magnitude to the next catalog
	if (star.mag() < _limit_mag) {
		return true;
	}
	_toofaint++;
	return false;
}

std::string	CutoverCondition::toString() const {
	std::string	msg = stringprintf(
		"%llu tried, %llu duplicates, %llu too faint",
		_count, _duplicates, _toofaint);
	if (_lowcut) {
		msg = msg + stringprintf(", %llu too bright", _toobright);
	}
	return msg;
}

//////////////////////////////////////////////////////////////////////
// BSCCondition implementation
//////////////////////////////////////////////////////////////////////
const float	BSCCondition::complete_mag = 4.5;

BSCCondition::BSCCondition(const float limit_mag)
	: CutoverCondition('\0', -30, limit_mag) {
}

BSCCondition::BSCCondition()
	: CutoverCondition('\0', -30, complete_mag) {
}

//////////////////////////////////////////////////////////////////////
// HipparcosCondition implementation
//////////////////////////////////////////////////////////////////////
const float	HipparcosCondition::complete_mag = 10.;

HipparcosCondition::HipparcosCondition(const float limit_mag,
	const float bsc_mag)
	: CutoverCondition('\0', bsc_mag, limit_mag) {
}

HipparcosCondition::HipparcosCondition(const float limit_mag)
	: CutoverCondition('\0', -30, limit_mag) {
}

HipparcosCondition::HipparcosCondition()
	: CutoverCondition('\0', -30, complete_mag) {
}

//////////////////////////////////////////////////////////////////////
// Tycho2Condition implementation
//////////////////////////////////////////////////////////////////////
const float	Tycho2Condition::complete_mag = 11;

Tycho2Condition::Tycho2Condition(float limit_mag, float hipparcos_mag)
	: CutoverCondition('H', hipparcos_mag, limit_mag) {
}

Tycho2Condition::Tycho2Condition(float limit_mag)
	: CutoverCondition('H', HipparcosCondition::complete_mag, limit_mag) {
}

Tycho2Condition::Tycho2Condition()
	: CutoverCondition('H', HipparcosCondition::complete_mag, complete_mag) {
}


//////////////////////////////////////////////////////////////////////
// Ucac4Condition implementation
//////////////////////////////////////////////////////////////////////
const float	Ucac4Condition::complete_mag = 16.;

Ucac4Condition::Ucac4Condition(float limit_mag, float tycho2_mag)
	: CutoverCondition('T', tycho2_mag, limit_mag) {
	lowcut(true);
}

Ucac4Condition::Ucac4Condition(float limit_mag)
	: CutoverCondition('T', Tycho2Condition::complete_mag, limit_mag) {
	lowcut(true);
}

Ucac4Condition::Ucac4Condition()
	: CutoverCondition('T', Tycho2Condition::complete_mag, complete_mag) {
	lowcut(true);
}

} // namespace catalog
} // namespace astro
