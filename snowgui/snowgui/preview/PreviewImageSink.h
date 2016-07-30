/*
 * PreviewImageSink.h -- an image sink that
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _PreviewImageSink_h
#define _PreviewImageSink_h

#include <camera.h>
#include <previewwindow.h>

class PreviewImageSink : public snowstar::ImageSink {
	PreviewWindow	*_preview;
public:
	PreviewImageSink(PreviewWindow *preview);
	virtual ~PreviewImageSink();
	void	image(const snowstar::ImageQueueEntry& entry,
			const Ice::Current& current);
	void	stop(const Ice::Current& current);
};

#endif  /* _PreviewImageSink_h */
