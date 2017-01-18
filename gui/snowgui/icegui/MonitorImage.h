/*
 * \brief MonitorImage.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _MonitorImage_h
#define _MonitorImage_h

#include <QObject>
#include <QLabel>
#include <guider.h>

namespace snowgui {

/**
 * \brief Callback monitor image class
 *
 * This class can be used as an image monitor in snowstar clients. It does
 * not know how to register or unregister, a derived class should add a method
 * to register the callback with the server and the destructor should be 
 * overridden to know how to unregister before the class goes out of scope
 */
class MonitorImage : public QObject, public snowstar::ImageMonitor {
	Q_OBJECT
	QLabel	*_label;
	QPixmap	*_pixmap;
	int	_scale;
	bool	_freeze;
	snowstar::SimpleImage	_image;
	void	rebuildImage();
protected:
	Ice::Identity	_myidentity;
public:
	MonitorImage(QObject *parent, QLabel *label);
	~MonitorImage();
	
	// interface methods for the callback
	virtual void	stop(const Ice::Current& current);
	virtual void	update(const snowstar::SimpleImage& image,
				const Ice::Current& current);

	// registering the callback with the adapter
	virtual void	do_register(Ice::ObjectPrx proxy,
				Ice::ObjectPtr myself);
	virtual void	do_unregister();

	// control image display
	int	scale() const { return _scale; }
	bool	freeze() const { return _freeze; }
signals:
	void	streamStopped();
	void	imageUpdated();
public slots:
	void	refreshImage();
	void	setScale(int s);
	void	setFreeze(bool f);
};

} // namespace snowgui

#endif /* _MonitorImage_h */
