/*
 * ImageLocator.h -- ImageLocator declarations
 *
 * (c) 2014 Prof DR Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ImageLocator_h
#define _ImageLocator_h

#include <Ice/Ice.h>
#include <map>
#include <ImageI.h>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace snowstar {

/**
 * \brief Image Locator class
 *
 * This class es used to locate ImageI objects. Since these objects
 * can consume large amounts of memory, we want to be able to tell them
 * to throw away the image they store. This is no problem because they
 * can reload the image from disk at any time. A separate thread is
 * used to expire images. The ImageI class has an expire() method that
 * causes it to throw away the image if it has not been accessed for some
 * time. This mitigates the impact of servants not beeing cleaned up by
 * clients for some time.
 */
class ImageLocator : public Ice::ServantLocator {
	typedef std::map<std::string, Ice::ObjectPtr>	imagemap;
	imagemap	_images;
	std::mutex	_mutex;
	std::condition_variable	_condition;
	std::thread	_thread;
private:
	ImageLocator(const ImageLocator& other);
	ImageLocator&	operator=(const ImageLocator& other);
public:
	ImageLocator();
	~ImageLocator();

	virtual Ice::ObjectPtr	locate(const Ice::Current& current,
			Ice::LocalObjectPtr& cookie);

	virtual void	finished(const Ice::Current& current,
				const Ice::ObjectPtr& servant,
				const Ice::LocalObjectPtr& cookie);

	virtual void	deactivate(const std::string& category);

	// methods related to expiration
private:
	bool	_stop;
public:
	void	stop();
	void	expire();
	void	run();
};

} // namespace snowstar

#endif /* _ImageLocator_h */
