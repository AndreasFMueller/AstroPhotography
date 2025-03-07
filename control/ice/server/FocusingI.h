/*
 * FocusingI.h -- implementation interface for focusing
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusingI_h
#define _FocusingI_h

#include <focusing.h>
#include <AstroFocus.h>
#include <CallbackHandler.h>
#include <RepositoryUser.h>
#include "StatisticsI.h"

namespace snowstar {

template<>
void	callback_adapter<FocusCallbackPrx>(FocusCallbackPrx p,
		const astro::callback::CallbackDataPtr d);

/**
 * \brief Focusing servant implementation
 */
class FocusingI : virtual public Focusing, virtual public RepositoryUser, public StatisticsI {
	astro::focusing::FocusingPtr	_focusingptr;
	FocusHistory	_history;
public:
	FocusingI(astro::focusing::FocusingPtr focusingptr);
	virtual ~FocusingI();

	FocusState	status(const Ice::Current& current);

	std::string	method(const Ice::Current& current);
	void	setMethod(const std::string& method,
			const Ice::Current& current);

	std::string	solver(const Ice::Current& current);
	void	setSolver(const std::string& solver,
			const Ice::Current& current);

	Exposure	getExposure(const Ice::Current& current);
	void	setExposure(const Exposure& exposure,
			const Ice::Current& current);

	int	steps(const Ice::Current& current);
	void	setSteps(int steps, const Ice::Current& current);

	void	start(int min, int max, const Ice::Current& current);
	void	cancel(const Ice::Current& current);

	CcdPrx	getCcd(const Ice::Current& current);
	FocuserPrx	getFocuser(const Ice::Current& current);

	FocusHistory	history(const Ice::Current& current);
	void	addPoint(const FocusPoint& point);

	virtual void	setRepositoryName(const std::string& reponame,
				const Ice::Current& current);
	virtual std::string	getRepositoryName(const Ice::Current& current);

	// callback stuff
private:
	SnowCallback<FocusCallbackPrx>	callbacks;
public:
	void	registerCallback(const Ice::Identity& callbackidentity,
			const Ice::Current& current);
	void	unregisterCallback(const Ice::Identity& callbackidentity,
			const Ice::Current& current);
	void	updateFocusing(const astro::callback::CallbackDataPtr data);
};

/**
 * \brief Callback class to be installed in the Focusing class
 *
 * The astro::focusing::Focusing class can accept a callback. An instance
 * of the FocusingCallback class can be installed there 
 */
class FocusingCallback : public astro::callback::Callback {
	FocusingI&	_focusing;
public:
	FocusingCallback(FocusingI& focusing) : _focusing(focusing) { }
	~FocusingCallback() { }
	virtual astro::callback::CallbackDataPtr	operator()(
		astro::callback::CallbackDataPtr data);
};

} // namespace snowstar

#endif /* _FocusingI_h */
