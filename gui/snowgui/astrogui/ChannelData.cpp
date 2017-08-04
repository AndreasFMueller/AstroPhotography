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

double	ChannelData::mean(double notbefore, double notafter) const {
	double	sum = 0;
	int	n = 0;
	ChannelData::const_iterator	i = begin();
	for (i = begin(); i != end(); i++) {
		if (notbefore <= i->time) {
			if (i->time <= notafter) {
				sum += i->value;
			} else {
				goto cleanup;
			}
		}
	}
cleanup:
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

double	ChannelData::var(double notbefore, double notafter) const {
	double	sum = 0;
	double	sum2 = 0;
	int	n = 0;
	ChannelData::const_iterator	i = begin();
	for (i = begin(); i != end(); i++) {
		if (notbefore <= i->time) {
			if (i->time <= notafter) {
				sum += i->value;
				sum2 += sqr(i->value);
			} else {
				goto cleanup;
			}
		}
	}
cleanup:
	return (n / (n-1)) * ((sum2 / n) - sqr(sum / n));
}

double	ChannelData::stddev() const {
	return sqrt(var());
}

double	ChannelData::stddev(int lastn) const {
	return sqrt(var(lastn));
}

double	ChannelData::stddev(double notbefore, double notafter) const {
	return sqrt(var(notbefore, notafter));
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

double	ChannelData::min(double notbefore, double notafter) const {
	double	m = std::numeric_limits<double>::infinity();
	ChannelData::const_iterator	i;
	for (i = begin(); i != end(); i++) {
		if (notbefore <= i->time) {
			if (i->time <= notafter) {
				if (i->value < m) {
					m = i->value;
				}
			} else {
				goto cleanup;
			}
		}
	}
cleanup:
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

double	ChannelData::max(double notbefore, double notafter) const {
	double	m = -std::numeric_limits<double>::infinity();
	ChannelData::const_iterator	i;
	for (i = begin(); i != end(); i++) {
		if (notbefore <= i->time) {
			if (i->time <= notafter) {
				if (i->value > m) {
					m = i->value;
				}
			} else {
				goto cleanup;
			}
		}
	}
cleanup:
	return m;
}

double	ChannelData::first() const {
	if (size() == 0) {
		return std::numeric_limits<double>::infinity();
	}
	return begin()->time;
}

double	ChannelData::last() const {
	if (size() == 0) {
		return -std::numeric_limits<double>::infinity();
	}
	return rbegin()->time;
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

std::vector<double>	ChannelDataVector::mean(double notbefore, double notafter) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,notbefore,notafter](const ChannelData& channel) mutable {
			result.push_back(channel.mean(notbefore, notafter));
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

std::vector<double>	ChannelDataVector::var(double notbefore, double notafter) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,notbefore,notafter](const ChannelData& channel) mutable {
			result.push_back(channel.var(notbefore, notafter));
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

std::vector<double>	ChannelDataVector::stddev(double notbefore, double notafter) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,notbefore,notafter](const ChannelData& channel) mutable {
			result.push_back(channel.stddev(notbefore, notafter));
		}
	);
	return result;
}

std::vector<double>	ChannelDataVector::min(int lastn) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,lastn](const ChannelData& channel) mutable {
			result.push_back(channel.min(lastn));
		}
	);
	return result;
}

std::vector<double>	ChannelDataVector::min(double notbefore, double notafter) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,notbefore,notafter](const ChannelData& channel) mutable {
			result.push_back(channel.min(notbefore, notafter));
		}
	);
	return result;
}

std::vector<double>	ChannelDataVector::max(int lastn) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,lastn](const ChannelData& channel) mutable {
			result.push_back(channel.max(lastn));
		}
	);
	return result;
}

std::vector<double>	ChannelDataVector::max(double notbefore, double notafter) const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result,notbefore,notafter](const ChannelData& channel) mutable {
			result.push_back(channel.max(notbefore, notafter));
		}
	);
	return result;
}

std::vector<double>	ChannelDataVector::last() const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result](const ChannelData& channel) mutable {
			result.push_back(channel.last());
		}
	);
	return result;
}

std::vector<double>	ChannelDataVector::first() const {
	std::vector<double>	result;
	std::for_each(begin(), end(),
		[&result](const ChannelData& channel) mutable {
			result.push_back(channel.first());
		}
	);
	return result;
}

double	ChannelDataVector::allMin(int lastn) const {
	std::vector<double>	minima = min(lastn);
	if (size() == 0) {
		return std::numeric_limits<double>::infinity();
	}
	double m = *min_element(minima.begin(), minima.end());
	return m;
}

double	ChannelDataVector::allMin(double notbefore, double notafter) const {
	std::vector<double>	minima = min(notbefore, notafter);
	if (size() == 0) {
		return std::numeric_limits<double>::infinity();
	}
	double m = *min_element(minima.begin(), minima.end());
	return m;
}

double	ChannelDataVector::allMax(int lastn) const {
	std::vector<double>	maxima = min(lastn);
	if (size() == 0) {
		return -std::numeric_limits<double>::infinity();
	}
	double m = *max_element(maxima.begin(), maxima.end());
	return m;
}

double	ChannelDataVector::allMax(double notbefore, double notafter) const {
	std::vector<double>	maxima = max(notbefore, notafter);
	if (size() == 0) {
		return std::numeric_limits<double>::infinity();
	}
	double m = *max_element(maxima.begin(), maxima.end());
	return m;
}

double	ChannelDataVector::allFirst() const {
	if (size() == 0) {
		return std::numeric_limits<double>::infinity();
	}
	std::vector<double>	f = first();
	return *min_element(f.begin(), f.end());
}

double	ChannelDataVector::allLast() const {
	if (size() == 0) {
		return -std::numeric_limits<double>::infinity();
	}
	std::vector<double>	l = last();
	return *max_element(l.begin(), l.end());
}

void	ChannelDataVector::clear() {
	std::for_each(begin(), end(), 
		[](ChannelData& channel) mutable { channel.clear(); }
	);
}

} // namespace snowgui

