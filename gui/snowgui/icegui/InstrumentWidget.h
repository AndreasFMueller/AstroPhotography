/*
 * InstrumentWidget.h -- base class for all widgets that need an instrument
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _InstrumentWidget_h
#define _InstrumentWidget_h

#include <QWidget>
#include <AstroDiscovery.h>
#include <RemoteInstrument.h>

namespace snowgui {

class InstrumentWidget : public QWidget {
	Q_OBJECT
	std::string	_appname;
	std::string	_title;
	astro::image::ImagePtr	_image;
public:
	void	setAppname(const std::string& appname);
	const std::string&	appname() const { return _appname; }
	astro::image::ImagePtr	image() { return _image; }
	void	image(astro::image::ImagePtr i) { _image = i; }
protected:
	astro::discover::ServiceKey	_servicekey;
	snowstar::RemoteInstrument	_instrument;
public:
	explicit InstrumentWidget(QWidget *parent = NULL);
	~InstrumentWidget();
	virtual void	instrumentSetup(
				astro::discover::ServiceObject serviceobject,
				snowstar::RemoteInstrument instrument);
	std::string	instrumentname();
	void	sendImage(astro::image::ImagePtr image, std::string title);
	void	changeEvent(QEvent *);

signals:
	void	offerImage(astro::image::ImagePtr, std::string);
};

} // namespace snowgui

#endif /* _InstrumentWidget_h */
