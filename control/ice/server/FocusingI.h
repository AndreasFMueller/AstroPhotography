/*
 * FocusingI.h -- implementation interface for focusing
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _FocusingI_h
#define _FocusingI_h

#include <focusing.h>

namespace snowstar {

class FocusingI : public Focusing {
public:
	FocusState	status(const Ice::Current& current);

	FocusMethod	method(const Ice::Current& current);
	void	setMethod(FocusMethod method, const Ice::Current& current);

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
};

} // namespace snowstar

#endif /* _FocusingI_h */
