/*
 * imagedetailwidget.cpp
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include "imagedetailwidget.h"
#include "ui_imagedetailwidget.h"
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <AstroIO.h>
#include <IceConversions.h>
#include <QFileDialog>
#include <QMessageBox>
#include <ImageForwarder.h>

namespace snowgui {

/**
 * \brief Construct a new widget
 */
imagedetailwidget::imagedetailwidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::imagedetailwidget) {
	ui->setupUi(this);

	// connections
	connect(ui->previewButton, SIGNAL(clicked()),
		this, SLOT(previewImage()));
	connect(ui->loadButton, SIGNAL(clicked()),
		this, SLOT(loadImage()));
	connect(ui->saveButton, SIGNAL(clicked()),
		this, SLOT(saveImage()));
	connect(ui->deleteButton, SIGNAL(clicked()),
		this, SLOT(deleteImage()));
	connect(this,
		SIGNAL(offerImage(astro::image::ImagePtr, std::string)),
		ImageForwarder::get(),
		SLOT(sendImage(astro::image::ImagePtr, std::string)));
}

/**
 * \brief Destroy the image widget
 */
imagedetailwidget::~imagedetailwidget() {
	delete ui;
}

/**
 * \brief Accept a new image proxy
 */
void	imagedetailwidget::setImage(snowstar::ImagePrx image) {
	_image = image;
	if (!_image) {
		return;
	}

	// process the new image proxy
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got an new image");
	std::string	s;
	try {
		s = _image->name();
	} catch (const std::exception& x) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot get image name: %s",
			x.what());
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "new image %s", s.c_str());

	ui->nameField->setText(QString(s.c_str()));

	time_t	now;
	time(&now);
	now -= _image->age();
	struct tm	*tmp = localtime(&now);
	char	buffer[100];
	strftime(buffer, sizeof(buffer), "%F", tmp);
	ui->dateField->setText(QString(buffer));
	strftime(buffer, sizeof(buffer), "%T", tmp);
	ui->timeField->setText(QString(buffer));

	ui->filesizeField->setText(QString::number(image->filesize()));

	snowstar::ImageSize	size = _image->size();
	s = astro::stringprintf("%d x %d", size.width, size.height);
	ui->sizeField->setText(QString(s.c_str()));

	snowstar::ImagePoint	origin = _image->origin();
	s = astro::stringprintf("(%d,%d)", origin.x, origin.y);
	ui->originField->setText(QString(s.c_str()));

	ui->planesField->setText(QString::number(_image->planes()));
	ui->bytespervalueField->setText(QString::number(_image->bytesPerPixel()));

	ui->loadButton->setEnabled(true);
	ui->previewButton->setEnabled(true);
	ui->deleteButton->setEnabled(true);
}

/**
 * \brief preview the image (JPEG transfer instead of FITS)
 */
void	imagedetailwidget::previewImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preview image %s", _image->name().c_str());
	snowstar::ImageBuffer	file = _image->file(snowstar::ImageEncodingJPEG);
	_imageptr = snowstar::convertimage(file);
	if (_imageptr) {
		ui->saveButton->setEnabled(false);
		emit offerImage(_imageptr, std::string());
	}
	emit imageReceived(_imageptr);
}

/**
 * \brief Load the image from the remote server
 */
void	imagedetailwidget::loadImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "load image %s", _image->name().c_str());
	snowstar::ImageBuffer	file = _image->file(snowstar::ImageEncodingFITS);
	_imageptr = snowstar::convertimage(file);
	if (_imageptr) {
		ui->saveButton->setEnabled(true);
		emit offerImage(_imageptr, std::string());
	}
	emit imageReceived(_imageptr);
}

/**
 * \brief Delete the image on the server side
 */
void	imagedetailwidget::deleteImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete image %s", _image->name().c_str());
	ui->nameField->setText(QString(""));
	ui->dateField->setText(QString(""));
	ui->timeField->setText(QString(""));
	ui->filesizeField->setText(QString(""));
	ui->sizeField->setText(QString(""));
	ui->originField->setText(QString(""));
	ui->planesField->setText(QString(""));
	ui->bytespervalueField->setText(QString(""));
	ui->previewButton->setEnabled(false);
	ui->loadButton->setEnabled(false);
	ui->saveButton->setEnabled(false);
	ui->deleteButton->setEnabled(false);
	emit deleteCurrentImage();
}

/**
 * \brief Save an image in the file system
 */
void	imagedetailwidget::saveImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "saving file");
	QFileDialog	filedialog(this);
	filedialog.setAcceptMode(QFileDialog::AcceptSave);
	filedialog.setFileMode(QFileDialog::AnyFile);
	filedialog.setDefaultSuffix(QString("fits"));
	if (filedialog.exec()) {
		QStringList	list = filedialog.selectedFiles();
		QStringList::const_iterator	i;
		std::string	filename(list.begin()->toLatin1().data());
		debug(LOG_DEBUG, DEBUG_LOG, 0, "filename: %s");
		astro::io::FITSout	out(filename);
		if (out.exists()) {
			out.unlink();
		}
		try {
			out.write(_imageptr);
		} catch (astro::io::FITSexception& x) {
			// find out whether file already exists
			QMessageBox	message(&filedialog);
			message.setText(QString("Save failed"));
			std::ostringstream	o;
			o << "Saving image to file '" << filename;
			o << "' failed. Cause: " << x.what();
			message.setInformativeText(QString(o.str().c_str()));
			message.exec();
		}
	}
}

/**
 * \brief handle the window close event
 */
void	imagedetailwidget::closeEvent(QCloseEvent * /* event */) {
        emit offerImage(ImagePtr(NULL), std::string());
}

} // namespace snowgui
