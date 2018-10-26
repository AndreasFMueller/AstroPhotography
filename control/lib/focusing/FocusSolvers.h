/*
 * FocusSolvers.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _FocusSolvers_h
#define _FocusSolvers_h

#include <AstroFocus.h>

namespace astro {
namespace focusing {

class CentroidSolver : public FocusSolver {
public:
	CentroidSolver();
	virtual ~CentroidSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class ParabolicSolver : public FocusSolver {
public:
	ParabolicSolver();
	virtual ~ParabolicSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class AbsoluteValueSolver : public ParabolicSolver {
public:
	AbsoluteValueSolver();
	virtual ~AbsoluteValueSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class MaximumSolver : public FocusSolver {
protected:
	float	maximum;
	float	minimum;
	int	maximumposition;
public:
	MaximumSolver();
	virtual ~MaximumSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class MinimumSolver : public FocusSolver {
protected:
	float	maximum;
	float	minimum;
	int	minimumposition;
public:
	MinimumSolver();
	virtual ~MinimumSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

class BrennerSolver : public MaximumSolver {
public:
	BrennerSolver();
	virtual ~BrennerSolver() { }
	virtual int	position(const FocusItems& focusitems);
};

} // namespace focusing
} // namespace astro

#endif /* _FocusSolver_h */
