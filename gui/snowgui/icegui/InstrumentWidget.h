/*
 * InstrumentWidget.h -- base class for all widgets that need an instrument
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _InstrumentWidget_h
#define _InstrumentWidget_h

#include <QWidget>
#include <QThread>
#include <AstroDiscovery.h>
#include <RemoteInstrument.h>

namespace snowgui {

class InstrumentWidget;

/**
 * \brief Thread class that can be used to 
 */
class InstrumentSetupThread : public QThread {
	Q_OBJECT
	InstrumentWidget		*_instrumentwidget;
	snowstar::RemoteInstrument		_remoteinstrument;
	astro::discover::ServiceObject	_serviceobject;
public:
	InstrumentSetupThread(InstrumentWidget *, snowstar::RemoteInstrument,
		astro::discover::ServiceObject);
	virtual ~InstrumentSetupThread();
	void	run();
signals:
	void	setupCompletion();
};

/**
 * \brief Base class for Instrument based widgets
 */
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
	void	launchInstrumentSetup(
				astro::discover::ServiceObject serviceobject,
				snowstar::RemoteInstrument instrument);
	virtual void	instrumentSetup(
				astro::discover::ServiceObject serviceobject,
				snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	std::string	instrumentname();
	void	sendImage(astro::image::ImagePtr image, std::string title);
	void	changeEvent(QEvent *);
public slots:
	void	setupCompletion();

signals:
	void	offerImage(astro::image::ImagePtr, std::string);
};

} // namespace snowgui

#endif /* _InstrumentWidget_h */
