/*
 * ChannelData.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <ChannelDisplayWidget.h>
#include <QPainter>
#include <AstroDebug.h>
#include <AstroFormat.h>
#include "ColorRectangles.h"

namespace snowgui {

static inline double	sqr(double x) { return x * x; }

//////////////////////////////////////////////////////////////////////
// Implementation of the ChannelData class
//////////////////////////////////////////////////////////////////////
void	ChannelData::push_back(const ChannelDataPoint& p) {
	if (p.value > _max) {
		_max = p.value;
	}
	if (p.value < _min) {
		_min = p.value;
	}
	_sum += p.value;
	_sum2 += sqr(p.value);
	std::deque<ChannelDataPoint>::push_back(p);
}

double	ChannelData::mean() const {
	return _sum / size();
}

double	ChannelData::mean(int lastn) const {
	double	sum = 0;
	int	n = 0;
	ChannelData::const_reverse_iterator	r = crbegin();
	while ((n < lastn) && (r != crend())) {
		sum += r->value;
		r++;
		n++;
	}
	return sum / n;
}

double	ChannelData::var() const {
	double	n = size();
	return (n / (n-1)) * ((_sum2 / n) - sqr(_sum / n));
}

double	ChannelData::var(int lastn) const {
	double	sum = 0;
	double	sum2 = 0;
	int	n = 0;
	ChannelData::const_reverse_iterator	r = crbegin();
	while ((n < lastn) && (r != crend())) {
		sum += r->value;
		sum2 += sqr(r->value);
		r++;
		n++;
	}
	return (n / (n - 1)) * ((sum2 / n) - sqr(sum / n));
}

double	ChannelData::stddev() const {
	return sqrt(var());
}

double	ChannelData::stddev(int lastn) const {
	return sqrt(var(lastn));
}

double	ChannelData::min(int lastn) const {
	double	m = std::numeric_limits<double>::infinity();
	int	counter = 0;
	ChannelData::const_reverse_iterator	r = crbegin();
	while ((counter < lastn) && (r != crend())) {
		if (r->value < m) {
			m = r->value;
		}
		r++;
		counter++;
	}
	return m;
}

double	ChannelData::max(int lastn) const {
	double	m = -std::numeric_limits<double>::infinity();
	int	counter = 0;
	ChannelData::const_reverse_iterator	r = crbegin();
	while ((counter < lastn) && (r != crend())) {
		if (r->value > m) {
			m = r->value;
		}
		r++;
		counter++;
	}
	return m;
}

//////////////////////////////////////////////////////////////////////
// ChannelDataVector implementation
//////////////////////////////////////////////////////////////////////
std::vector<double>	ChannelDataVector::mean(int lastn) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,lastn](const ChannelData& channel) mutable {
			result.push_back(channel.mean(lastn));
		}
	);
	return result;
}

std::vector<double>	ChannelDataVector::var(int lastn) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,lastn](const ChannelData& channel) mutable {
			result.push_back(channel.var(lastn));
		}
	);
	return result;
}

std::vector<double>	ChannelDataVector::stddev(int lastn) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,lastn](const ChannelData& channel) mutable {
			result.push_back(channel.stddev(lastn));
		}
	);
	return result;
}

} // namespace snowgui

