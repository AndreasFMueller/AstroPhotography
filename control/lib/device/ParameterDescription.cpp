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
	virtual ~ParameterDescriptionImpl() {
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

	virtual bool	get_boolean() const {
		std::string	msg = stringprintf("cannot get boolean from %s",
			demangle_string(*this).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
		throw std::runtime_error("no boolean value");
	}
	virtual float	get_float() const {
		std::string	msg = stringprintf("cannot get float from %s",
			demangle_string(*this).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	virtual std::string	get_string() const {
		std::string	msg = stringprintf("cannot get string from %s",
			demangle_string(*this).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

	virtual void	set_boolean(bool v) {
		std::string	msg = stringprintf("cannot set boolean(%s) of %s",
			(v) ? "true" : "false",
			demangle_string(*this).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	virtual void	set_float(float v) {
		std::string	msg = stringprintf("cannot set float(%f) of %s",
			v, demangle_string(*this).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
		throw std::runtime_error("cannot set float value");
	}
	virtual void	set_string(const std::string& s) {
		std::string	msg = stringprintf("cannot set string(%s) of %s",
			s.c_str(), demangle_string(*this).c_str());
		debug(LOG_ERR, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
		throw std::runtime_error("cannot set string value");
	}

};

//////////////////////////////////////////////////////////////////////
// ParameterDescription for boolean parameters
//////////////////////////////////////////////////////////////////////
class ParameterDescriptionImplBoolean : public ParameterDescriptionImpl {
	bool	_value;
public:
	ParameterDescriptionImplBoolean() : ParameterDescriptionImpl() {
		_value = false;
	}
	virtual ~ParameterDescriptionImplBoolean() {
	}
	virtual bool	isvalid(const std::string& value) const {
		return (value == "true") || (value == "false");
	}
	virtual bool	isvalid(const float& value) const {
		return (value) || (!(value));
	}
	virtual void	set_boolean(bool v) {
		_value = v;
	}
	virtual void	set_string(const std::string& v) {
		_value = (v == "true");
	}
	virtual bool	get_boolean() const {
		return _value;
	}
	virtual std::string	get_string() const {
		return (_value) ? std::string("true") : std::string("false");
	}
};


//////////////////////////////////////////////////////////////////////
// ParameterDescription for range parameters
//////////////////////////////////////////////////////////////////////
class ParameterDescriptionImplRange : public ParameterDescriptionImpl {
	float	_value;
protected:
	float	_from, _to;
public:
	ParameterDescriptionImplRange(float from, float to)
		: _from(from), _to(to) { }
	virtual ~ParameterDescriptionImplRange() { }
	virtual bool	isvalid(const float& value) const {
		return (_from <= value) && (value <= _to);
	}
	virtual float	from() const { return _from; }
	virtual float	to() const { return _to; }
	virtual float	get_float() const {
		return _value;
	}
	virtual void	set_float(float f) {
		if (!(this->isvalid(f))) {
			throw std::range_error("invalid float parameter");
		}
		_value = f;
	}
	virtual std::string	get_string() const {
		return stringprintf("%f", _value);
	}
	virtual void	set_string(const std::string& s) {
		set_float(std::stof(s));
	}
};

//////////////////////////////////////////////////////////////////////
// ParameterDescription for sequence parameters
//////////////////////////////////////////////////////////////////////
class ParameterDescriptionImplSequence : public ParameterDescriptionImplRange {
	float	_step;
public:
	ParameterDescriptionImplSequence(float from, float to, float step)
		: ParameterDescriptionImplRange(from, to), _step(step) { }
	virtual ~ParameterDescriptionImplSequence() { }
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
	T	_value;
protected:
	std::set<T>	_values;
public:
	ParameterDescriptionImplSet(const std::set<T>& values)
		: _values(values) { }
	ParameterDescriptionImplSet(const std::vector<T>& values) {
		copy(values.begin(), values.end(),
			std::inserter(_values, _values.begin()));
	}
	virtual ~ParameterDescriptionImplSet() { }
	virtual bool	isvalid(const T& value) const {
		return (_values.end() != _values.find(value));
	}
	void	add(const T& value) {
		_values.insert(value);
	}
protected:
	T	get() const {
		return _value;
	}
	void	set(T v) {
		if (!(this->isvalid(v))) {
			throw std::runtime_error("invalid parameter value");
		}
		_value = v;
	}
};

/**
 * \brief Additiobnal methods for processing of float values
 */
class ParameterDescriptionImplSetFloat
	: public ParameterDescriptionImplSet<float> {
public:
	ParameterDescriptionImplSetFloat(const std::set<float> values)
		: ParameterDescriptionImplSet<float>(values) { }
	ParameterDescriptionImplSetFloat(const std::vector<float> values)
		: ParameterDescriptionImplSet<float>(values) { }
	virtual ~ParameterDescriptionImplSetFloat() { }
	// using directive to silence clang warning
	using ParameterDescriptionImplSet<float>::add;
	void	add(const std::string& value) {
		ParameterDescriptionImplSet<float>::add(std::stod(value));
	}
	bool	isvalid(const float& value) const;
	virtual std::set<float>	floatValues() const { return _values; }

	virtual float	get_float() const { return get(); }
	virtual void	set_float(float f) { set(f); }
};

/**
 * \brief The ClosesValues class
 *
 * This class constructs a map of distances of values from a given
 * reference value. Then the operator[] allows to access the pair
 * of distance/value ordered by distance, so operator[](0) gives
 * the pair with smallest distance from the reference value
 */
class ClosestValues {
	size_t	_n;
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
	ClosestValues(const ClosestValues& other) = delete;
	ClosestValues&	operator=(const ClosestValues& other) = delete;
public:
	ClosestValues(size_t n, float reference, const std::set<float>& v)
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
	std::pair<float, float>	operator[](size_t index) const {
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
	ParameterDescriptionImplSetString(const std::vector<std::string> values)
		: ParameterDescriptionImplSet<std::string>(values) { }
	// using directive to silence clang warning
	using ParameterDescriptionImplSet<std::string>::isvalid;
	virtual bool	isvalid(const float& value) const {
		std::string	s = convert(value);
		return ParameterDescriptionImplSet<std::string>::isvalid(s);
	}
	// using directive to silence clang warning
	using ParameterDescriptionImplSet<std::string>::add;
	void	add(const float& value) {
		std::string	s = convert(value);
		ParameterDescriptionImplSet<std::string>::add(s);
	}
	virtual std::set<std::string>	stringValues() const {
		return _values;
	}

	virtual std::string	get_string() const {
		return get();
	}
	virtual void	set_string(const std::string& s) {
		set(s);
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
	const std::vector<float>& values)
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

ParameterDescription::ParameterDescription(const std::string& name,
	const std::vector<std::string>& values)
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

bool	ParameterDescription::get_boolean() const {
	return _impl->get_boolean();
}

float	ParameterDescription::get_float() const {
	return _impl->get_float();
}

std::string	ParameterDescription::get_string() const {
	return _impl->get_string();
}

void	ParameterDescription::set_boolean(bool v) {
	_impl->set_boolean(v);
}

void	ParameterDescription::set_float(float f) {
	_impl->set_float(f);
}

void	ParameterDescription::set_string(const std::string& s) {
	_impl->set_string(s);
}

} // namespace device
} // namespace astro
