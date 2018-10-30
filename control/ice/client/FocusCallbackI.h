/*
 * FocusCallbackI.h
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusCallbackI_h
#define _FocusCallbackI_h

#include <focusing.h>
#include <typeinfo>
#include <AstroUtils.h>
#include <AstroConfig.h>

using namespace astro;
using namespace snowstar;

namespace snowstar {
namespace app {
namespace snowfocus {

/**
 * \brief Callback class for the snowfocus program
 *
 * This callback simply displays the callback information received
 */
class FocusCallbackI : public FocusCallback {
	std::string	_raw_prefix;
	std::string	_evaluated_prefix;
public:
	void	raw_prefix(const std::string raw_prefix);
	void	evaluated_prefix(const std::string& evaluated_prefix);
	FocusCallbackI();
	void	addPoint(const FocusPoint& point, const Ice::Current& current);
	void	changeState(FocusState state, const Ice::Current& current);
	void	addFocusElement(const FocusElement& element,
			const Ice::Current& current);
};

} // namespace snowfocus
} // namespace app
} // namespace snowstar

#endif /* _FocusCallbackI_h */
