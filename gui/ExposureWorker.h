/*
 * ExposureWorker.h -- worker thread to do actual exposing/decoding of
 *                     images
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _ExposureWorker_h
#define _ExposureWorker_h

#include <QObject>
#include <AstroImage.h>
#include <AstroCamera.h>

using namespace astro::camera;
using namespace astro::image;

class CaptureWindow;

class ExposureWorker : public QObject {
	Q_OBJECT
	CcdPtr	ccd;
	Exposure	exposure;
	CaptureWindow	*capturewindow;

public:
	ExposureWorker(CcdPtr ccd, const Exposure& exposure,
		CaptureWindow *capturewindow, QObject *parent = 0);
	virtual ~ExposureWorker();

signals:
	void	finished();

public slots:
	void	process();

};

#endif /* _ExposureWorker_h */
