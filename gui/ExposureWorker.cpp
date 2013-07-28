/*
 * ExposureWorker.cpp -- ExposureWorker implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <ExposureWorker.h>
#include <capturewindow.h>

ExposureWorker::ExposureWorker(CcdPtr _ccd, const Exposure& _exposure,
	CaptureWindow *_capturewindow,
	QObject *parent) : QObject(parent), ccd(_ccd), exposure(_exposure),
		capturewindow(_capturewindow) {
}

ExposureWorker::~ExposureWorker() {
}

void	ExposureWorker::process() {
	ccd->startExposure(exposure);
	ImagePtr	image = ccd->getImage();
	capturewindow->newImage(image);
	emit finished();
}

