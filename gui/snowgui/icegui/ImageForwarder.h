/*
 * ImageForwarder.h
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _ImageForwarder_h
#define _ImageForwarder_h

#include <QObject>
#include <AstroImage.h>

namespace snowgui {

class ImageForwarder : public QObject {
	Q_OBJECT

public:
	ImageForwarder();

static ImageForwarder*	get();

signals:
	void	offerImage(astro::image::ImagePtr, std::string);

public slots:
	void	sendImage(astro::image::ImagePtr, std::string);
};

} // namespace snowgui

#endif /* _ImageForwarder_h */
