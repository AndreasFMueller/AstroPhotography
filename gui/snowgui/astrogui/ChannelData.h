/*
 * ChannelData.h -- Data to display in the ChannelDisplayWidget
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ChannelData_h
#define _ChannelData_h

#include <deque>
#include <limits>
#include <AstroUtils.h>

namespace snowgui {

/**
 * \brief A single data point in a ChannelData object
 */
class ChannelDataPoint {
public:
	double	time;
	double	value;
	ChannelDataPoint(double t, double v) : time(t), value(v) { }
	ChannelDataPoint(double v) : time(astro::Timer::gettime()), value(v) { }
	ChannelDataPoint() : time(astro::Timer::gettime()), value(0) { }
};

/**
 * \brief A Channel of data to be displayed by the ChannelDisplayWidget
 */
class ChannelData : public std::deque<ChannelDataPoint> {
	double	_sum;
	double	_sum2;
	double	_min;
	double	_max;
public:
	ChannelData() : _sum(0), _sum2(0),
		_min(std::numeric_limits<double>::infinity()),
		_max(-std::numeric_limits<double>::infinity()) { }

	double	min() const { return _min; }
	double	min(int lastn) const;
	double	min(double notbefore, double notafter) const;

	double	max() const { return _max; }
	double	max(int lastn) const;
	double	max(double notbefore, double notafter) const;

	double	mean() const;
	double	mean(int lastn) const;
	double	mean(double notbefore, double notafter) const;

	double	var() const;
	double	var(int lastn) const;
	double	var(double notbefore, double notafter) const;

	double	stddev() const;
	double	stddev(int lastn) const;
	double	stddev(double notbefore, double notafter) const;

	double	last() const;
	double	first() const;

	void	push_back(const ChannelDataPoint& p);
};

/**
 * \brief Vector of ChannelData
 *
 * This object is used as the data container for the ChannelDisplayWidget
 */
class ChannelDataVector : public std::vector<ChannelData> {
public:
	std::vector<double>	min() const;
	std::vector<double>	min(int lastn) const;
	std::vector<double>	min(double notbefore, double notafter) const;

	std::vector<double>	max() const;
	std::vector<double>	max(int lastn) const;
	std::vector<double>	max(double notbefore, double notafter) const;

	std::vector<double>	mean() const;
	std::vector<double>	mean(int lastn) const;
	std::vector<double>	mean(double notbefore, double notafter) const;

	std::vector<double>	var() const;
	std::vector<double>	var(int lastn) const;
	std::vector<double>	var(double notbefore, double notafter) const;

	std::vector<double>	stddev() const;
	std::vector<double>	stddev(int lastn) const;
	std::vector<double>	stddev(double notbefore, double notafter) const;

	std::vector<double>	last() const;
	std::vector<double>	first() const;

	double	allMin() const;
	double	allMin(int lastn) const;
	double	allMin(double notbefore, double notafter) const;

	double	allMax() const;
	double	allMax(int lastn) const;
	double	allMax(double notbefore, double notafter) const;

	double	allLast() const;
	double	allFirst() const;

	void	clear();
};

} // namespace snowgui

#endif /* _ChannelData_h */
