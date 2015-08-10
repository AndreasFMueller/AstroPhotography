/*
 * ParameterDescription.cpp -- description of parameters
 *
 * (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroDevice.h>
#include <sstream>
#include <set>
#include <AstroDebug.h>

namespace astro {
namespace device {

//////////////////////////////////////////////////////////////////////
// base class for implementations
//////////////////////////////////////////////////////////////////////
class ParameterDescriptionImpl {
public:
	ParameterDescriptionImpl() {
	}
	virtual bool	isvalid(const std::string& value) const {
		try {
			return isvalid(std::stod(value));
		} catch (...) {
		}
		return false;
	}
	virtual bool	isvalid(const float& value) const = 0;
	virtual void	add(const std::string& /* value */) {
		throw std::logic_error("cannot add to this type of parameter");
	}
	virtual void	add(const float& /* value */) {
		throw std::logic_error("cannot add to this type of parameter");
	}
	virtual float	from() const {
		throw std::logic_error("cannot get from()");
	}
	virtual float	to() const {
		throw std::logic_error("cannot get to()");
	}
	virtual float	step() const {
		throw std::logic_error("cannot get step()");
	}
	virtual std::set<float>	floatValues() const {
		throw std::logic_error("cannot get floatValues()");
	}
	virtual std::set<std::string>	stringValues() const {
		throw std::logic_error("cannot get stringValues()");
	}
};

//////////////////////////////////////////////////////////////////////
// ParameterDescription for boolean parameters
//////////////////////////////////////////////////////////////////////
class ParameterDescriptionImplBoolean : public ParameterDescriptionImpl {
public:
	ParameterDescriptionImplBoolean() : ParameterDescriptionImpl() { }
	virtual bool	isvalid(const std::string& value) const {
		return (value == "true") || (value == "false");
	}
	virtual bool	isvalid(const float& value) const {
		return (value) || (!(value));
	}
};


//////////////////////////////////////////////////////////////////////
// ParameterDescription for range parameters
//////////////////////////////////////////////////////////////////////
class ParameterDescriptionImplRange : public ParameterDescriptionImpl {
protected:
	float	_from, _to;
public:
	ParameterDescriptionImplRange(float from, float to)
		: _from(from), _to(to) { }
	virtual bool	isvalid(const float& value) const {
		return (_from <= value) && (value <= _to);
	}
	virtual float	from() const { return _from; }
	virtual float	to() const { return _to; }
};

//////////////////////////////////////////////////////////////////////
// ParameterDescription for sequence parameters
//////////////////////////////////////////////////////////////////////
class ParameterDescriptionImplSequence : public ParameterDescriptionImplRange {
	float	_step;
public:
	ParameterDescriptionImplSequence(float from, float to, float step)
		: ParameterDescriptionImplRange(from, to), _step(step) { }
	virtual bool	isvalid(const float& value) const {
		if (!ParameterDescriptionImplRange::isvalid(value)) {
			return false;
		}
		// check that value is close to a multiple of the step size
		float	v = (value - _from) / _step;
		return (fabs(v - round(v)) < 0.01);
	}
	virtual float	step() const { return _step; }
};

//////////////////////////////////////////////////////////////////////
// ParameterDescription for set parameters
//////////////////////////////////////////////////////////////////////
template<typename T>
class ParameterDescriptionImplSet : public ParameterDescriptionImpl {
protected:
	std::set<T>	_values;
public:
	ParameterDescriptionImplSet(const std::set<T>& values)
		: _values(values) { }
	virtual bool	isvalid(const T& value) const {
		return (_values.end() != _values.find(value));
	}
	virtual void	add(const T& value) {
		_values.insert(value);
	}
};

class ParameterDescriptionImplSetFloat
	: public ParameterDescriptionImplSet<float> {
public:
	ParameterDescriptionImplSetFloat(const std::set<float> values)
		: ParameterDescriptionImplSet<float>(values) { }
	virtual void	add(const std::string& value) {
		ParameterDescriptionImplSet<float>::add(std::stod(value));
	}
	virtual bool	isvalid(const float& value) const;
	virtual std::set<float>	floatValues() const { return _values; }
};

class ClosestValues {
	int	_n;
	float	_reference;
	typedef	std::map<float, float>	valuemap_t;
	valuemap_t	_values;
	void	removelast() {
		valuemap_t::const_iterator	i = _values.begin();
		int	j = _n;
		while (0 < j--) {
			i++;
		}
		_values.erase(i);
	}
private:
	ClosestValues(const ClosestValues& other);
	ClosestValues&	operator=(const ClosestValues& other);
public:
	ClosestValues(int n, float reference, const std::set<float>& v)
		: _n(n), _reference(reference) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "construct ClosestValues");
		std::set<float>::const_iterator	i;
		for (i = v.begin(); i != v.end(); i++) {
			double	value = *i;
			float	distance = fabs(value - _reference);
			_values.insert(std::make_pair(distance, value));
			if (_values.size() > _n) {
				removelast();
			}
		}
	}
	std::pair<float, float>	operator[](int index) const {
		if (index >= _n) {
			throw std::range_error("index exceeds size");
		}
		valuemap_t::const_iterator	i = _values.begin();
		while (0 < index--) {
			if (_values.end() == i) {
				throw std::range_error("not enough data");
			}
			i++;
		}
		return *i;
	}
};

bool	ParameterDescriptionImplSetFloat::isvalid(const float& value) const {
	if (0 == _values.size()) {
		return false;
	}
	// in case the value is contained exactly in the 
	if (ParameterDescriptionImplSet<float>::isvalid(value)) {
		return true;
	}
	// if we have a single value, we just assume that if we are closer
	// than 1ppm, then 
	if (1 == _values.size()) {
		float	v = *_values.begin();
		return (fabs(v) < 1e6 * fabs(value - v));
	}
	
	// find the distance to the two closes values from the set
	debug(LOG_DEBUG, DEBUG_LOG, 0, "checking %f", value);
	ClosestValues	closest(2, value, _values);
	std::pair<float, float>	s1 = closest[0];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "closest: %f (%f)", s1.second, s1.first);
	std::pair<float, float>	s2 = closest[1];
	debug(LOG_DEBUG, DEBUG_LOG, 0, "next: %f (%f)", s2.second, s2.first);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "distance between closest values: %f",
		fabs(s2.second - s1.second));
	if (s1.first < 0.001 * s2.first) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"accepting the value 1000 times closer");
		return true;
	}
	return false;
}

class ParameterDescriptionImplSetString
	: public ParameterDescriptionImplSet<std::string> {
	std::string	convert(const float& value) const {
		std::ostringstream	out;
		out << value;
		return out.str();
	}
public:
	ParameterDescriptionImplSetString(const std::set<std::string> values)
		: ParameterDescriptionImplSet<std::string>(values) { }
	virtual bool	isvalid(const float& value) const {
		std::string	s = convert(value);
		return ParameterDescriptionImplSet<std::string>::isvalid(s);
	}
	virtual void	add(const float& value) {
		std::string	s = convert(value);
		ParameterDescriptionImplSet<std::string>::add(s);
	}
	virtual std::set<std::string>	stringValues() const {
		return _values;
	}
};

//////////////////////////////////////////////////////////////////////
// ParameterDescription implementation
//////////////////////////////////////////////////////////////////////

ParameterDescription::ParameterDescription(const std::string& name)
	: _name(name), _type(ParameterDescription::boolean) {
	_impl = ParameterDescriptionImplPtr(
			new ParameterDescriptionImplBoolean());
}

ParameterDescription::ParameterDescription(const std::string& name,
	float from, float to)
	: _name(name), _type(ParameterDescription::range) {
	_impl = ParameterDescriptionImplPtr(
			new ParameterDescriptionImplRange(from, to));
}

ParameterDescription::ParameterDescription(const std::string& name,
	float from, float to, float step)
	: _name(name), _type(ParameterDescription::sequence) {
	_impl = ParameterDescriptionImplPtr(
			new ParameterDescriptionImplSequence(from, to, step));
}

ParameterDescription::ParameterDescription(const std::string& name,
	const std::set<float>& values)
	: _name(name), _type(ParameterDescription::floatset) {
	_impl = ParameterDescriptionImplPtr(
			new ParameterDescriptionImplSetFloat(values));
}

ParameterDescription::ParameterDescription(const std::string& name,
	const std::set<std::string>& values)
	: _name(name), _type(ParameterDescription::stringset) {
	_impl = ParameterDescriptionImplPtr(
			new ParameterDescriptionImplSetString(values));
}

bool	ParameterDescription::isvalid(const std::string& value) const {
	return _impl->isvalid(value);
}

bool	ParameterDescription::isvalid(const float& value) const {
	return _impl->isvalid(value);
}

void	ParameterDescription::add(const std::string& value) {
	_impl->add(value);
}

void	ParameterDescription::add(const float& value) {
	_impl->add(value);
}

float	ParameterDescription::from() const {
	return _impl->from();
}

float	ParameterDescription::to() const {
	return _impl->to();
}

float	ParameterDescription::step() const {
	return _impl->step();
}

std::set<float>	ParameterDescription::floatValues() const {
	return _impl->floatValues();
}

std::set<std::string>	ParameterDescription::stringValues() const {
	return _impl->stringValues();
}

} // namespace device
} // namespace astro
