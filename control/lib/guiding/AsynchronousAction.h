/*
 * AsynchronousAction.h -- a class that encapsulates a function call
 *			   and executes it asynchronously
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _AsynchronousAction_h
#define _AsynchronousAction_h

#include <memory>
#include <thread>

namespace astro {
namespace guiding {

class Action {
public:
	virtual void	execute() = 0;
};
typedef std::shared_ptr<Action>	ActionPtr;

class AsynchronousAction {
	std::thread	worker;
	ActionPtr	_action;
	bool		_busy;
	std::mutex	mtx;
private:
	AsynchronousAction(const AsynchronousAction& other);
	AsynchronousAction&	operator=(const AsynchronousAction& other);
public:
	void	busy(bool b);
	AsynchronousAction();
	~AsynchronousAction();
	bool	execute(ActionPtr action);
	void	execute();
	
};

} // namespace guiding
} // namespace astro

#endif /* _AsynchronousAction_h */
