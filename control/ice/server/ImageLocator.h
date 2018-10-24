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
#include <EvictorBase.h>

namespace snowstar {

/**
 * \brief Image Locator class
 *
 * This class es used to locate ImageI objects. Since these objects
 * can consume large amounts of memory, we want to be able to tell them
 * to throw away the image they store. This is no problem because they
 * can reload the image from disk at any time.
 */
class ImageLocator : public EvictorBase {
private:
	ImageLocator(const ImageLocator& other);
	ImageLocator&	operator=(const ImageLocator& other);
public:
	ImageLocator();
	virtual ~ImageLocator();

protected:
	virtual Ice::ObjectPtr	add(const Ice::Current& current,
					Ice::LocalObjectPtr& cookie);
	virtual void	evict(const Ice::ObjectPtr& object,
				const Ice::LocalObjectPtr& cookie);
};

} // namespace snowstar

#endif /* _ImageLocator_h */
