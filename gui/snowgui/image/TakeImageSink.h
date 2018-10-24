/*
 * TakeImageSink.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _TakeImageSink_h
#define _TakeImageSink_h

#include <camera.h>
#include <QObject>
#include <AstroImage.h>
#include <IceConversions.h>

namespace snowgui {

class TakeImageSink : public QObject, public snowstar::ImageSink {
	Q_OBJECT
	bool	_enabled;
public:
	explicit TakeImageSink(QObject *parent = NULL);
	virtual ~TakeImageSink();

	// methods for the ImageSink
	void	image(const snowstar::ImageQueueEntry& entry,
			const Ice::Current& current);
	void	stop(const Ice::Current& current);

signals:
	void	newImage(astro::image::ImagePtr);
	void	finished();

public slots:
	void	setEnabled(bool);
};

} // namespace snowgui

#endif /* _TakeImageSink_h */
