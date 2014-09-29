/*
 * FocusingLocator.cpp
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <FocusingLocator.h>
#include <exceptions.h>

namespace snowstar {


FocusingLocator::FocusingLocator() {
}

void	FocusingLocator::add(const int id, Ice::ObjectPtr focusingptr) {
	focusings.insert(std::make_pair(id, focusingptr));
}

Ice::ObjectPtr  FocusingLocator::locate(const Ice::Current& current,
		Ice::LocalObjectPtr& cookie) {
	int	id = std::stoi(cookie.id.name);
	if (focusings.find(id) == focusings.end()) {
		throw NotFound();
	}
	return focusings.find(id)->second;
}

void	FocusingLocator::finished(const Ice::Current& current,
			const Ice::ObjectPtr& servant,
			const Ice::LocalObjectPtr& cookie) {
}

void	FocusingLocator::deactivate(const std::string& category) {
}

} // namespace snowstar

#endif /* _FocusingLocator_h */

} // namespace snowstar
