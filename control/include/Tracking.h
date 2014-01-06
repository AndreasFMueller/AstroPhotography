/*
 * Tracking.h -- Table containing tracking log data
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Tracking_h
#define _Tracking_h

#include <string>
#include <AstroPersistence.h>
#include <AstroGuiding.h>

namespace astro {
namespace guiding {

/**
 * \brief Encapsulation of the information about a guide run
 */
class GuidingRun {
	int	_id;
public:
	int	id() const { return _id; }
	void	id(int i) { _id = i; }
	time_t	whenstarted;
	std::string	camera;
	int	ccdid;
	std::string	guiderport;
	GuidingRun() { }
};

/**
 * \brief Adapter for GuidingRun table entries
 */
class GuidingRunTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static GuidingRun
	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec
	object_to_updatespec(const GuidingRun& guidingrun);
};

typedef astro::persistence::Table<GuidingRun, GuidingRunTableAdapter>	GuidingRunTable;

/**
 * \brief Encapsulation of
 */
class Tracking {
	int	_id;
public:
	int	id() const { return _id; }
	void	id(int i) { _id = i; }
	int	guidingrun; // references GuiderRun.id
	double	when;
	double	xoffset;
	double	yoffset;
	double	racorrection;
	double	deccorrection;
	
	Tracking(int i) : _id(i) { }
	Tracking(int i, const int& _guidingrun, const TrackingInfo& trackinfo)
		: _id(i), guidingrun(_guidingrun), when(trackinfo.t),
		  xoffset(trackinfo.trackingoffset.x()),
		  yoffset(trackinfo.trackingoffset.y()),
		  racorrection(trackinfo.correction.x()),
		  deccorrection(trackinfo.correction.y()) { }
};

/**
 * \brief Adapter for the Tracking table
 */
class TrackingTableAdapter {
public:
static std::string	tablename();
static std::string	createstatement();
static Tracking	row_to_object(int objectid, const astro::persistence::Row& row);
static astro::persistence::UpdateSpec	object_to_updatespec(const Tracking& tracking);
};

typedef astro::persistence::Table<Tracking, TrackingTableAdapter>	TrackingTable;

} // namespace guiding
} // namespace astro

#endif /* _Tracking_h */
