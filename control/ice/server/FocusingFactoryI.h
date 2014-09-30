/*
 * FocusingFactoryI.h
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusingFactoryI_h
#define _FocusingFactoryI_h

#include <string>
#include <AstroFocus.h>
#include <focusing.h>

namespace snowstar {

/**
 * \brief key class for teh focusing context map in the factory
 */
class FocusingKey : public std::pair<std::string, std::string> {
public:
	FocusingKey(const std::string ccd, const std::string focuser)
		: std::pair<std::string, std::string>(ccd, focuser) { }
	const std::string&	ccd() const { return first; }
	std::string&	ccd() { return first; }
	void	ccd(const std::string& c) { first = c; }
	const std::string	focuser() const { return second; }
	std::string&	focuser() { return second; }
	void	focuser(const std::string& f) { second = f; }
	std::string	toString() const;
};

/**
 * \brief Holder class for focusing context
 */
class FocusingContext {
public:
	int	id;
	astro::focusing::FocusingPtr	focusing;
	Ice::ObjectPtr	focusingptr;
};

/**
 * \brief Backend factory implementation
 *
 * This class implements a singleton to access the focusing contexts in a
 * map. Note that the implementation has to make sure access to the map is
 * exlusive, but we don't show this implementation detail in this header.
 */
class FocusingSingleton {
public:
	typedef std::map<FocusingKey, FocusingContext>	FocusingMap;
private:
static FocusingMap	focusings;
public:
static FocusingContext	get(const std::string& ccd, const std::string& focuser);
static FocusingContext	get(int id);
};

/**
 * \brief Factory for Focusing proxies
 *
 * This factory method just looks up an entry in th focusing map, and if none
 * is found, creates a new one. The id in the FocusingContext is then used
 * to create a string identifier for the Ice proxy.
 */
class FocusingFactoryI : public FocusingFactory {
public:
	FocusingFactoryI();
	virtual ~FocusingFactoryI();
	FocusingPrx	get(const std::string& ccd, const std::string& focuser,
				const Ice::Current& current);
};

} // namespace snowstar

#endif /* _FocusingFactoryI_h */

