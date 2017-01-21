/*
 * \brief imageswindow.cpp -- implementation of images preview
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */ 
#include "imageswindow.h"
#include "ui_imageswindow.h"
#include <AstroDebug.h>
#include <CommunicatorSingleton.h>
#include "ImageInfo.h"
#include <QTreeWidgetItem>

namespace snowgui {

/**
 * \brief Construct an images window
 */

imageswindow::imageswindow(QWidget *parent,
	astro::discover::ServiceObject serviceobject)
	: QSplitter(parent), ui(new Ui::imageswindow),
	  _serviceobject(serviceobject) {
	ui->setupUi(this);

	// don't show the subframe information
	QStringList	headers;
        headers << "Date" << "Time" << "Size" << "Filename";
        ui->imageTree->setHeaderLabels(headers);
        ui->imageTree->header()->resizeSection(0, 120);
	ui->imageTree->header()->resizeSection(1, 80);
        ui->imageTree->header()->resizeSection(2, 80);

	// connect to the Images service
	Ice::CommunicatorPtr	ic = snowstar::CommunicatorSingleton::get();
	Ice::ObjectPrx	base = ic->stringToProxy(
			_serviceobject.connect("Images"));
        snowstar::ImagesPrx      images
		= snowstar::ImagesPrx::checkedCast(base);
        if (!base) {
                throw std::runtime_error("cannot create configuration app");
        }
        setImages(images);

	// set the window title
	std::string	title = astro::stringprintf("Images on %s",
		_serviceobject.toString().c_str());
	setWindowTitle(QString(title.c_str()));

	// add connections
	connect(ui->imageTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
		this, SLOT(currentImageChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(ui->imageTree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
		this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));
	connect(ui->imagedetailWidget, SIGNAL(imageReceived(astro::image::ImagePtr)),
		this, SLOT(setImage(astro::image::ImagePtr)));
	connect(ui->imagedetailWidget, SIGNAL(deleteCurrentImage()),
		this, SLOT(deleteCurrentImage()));

	connect(ui->imageWidget, SIGNAL(rectangleSelected(astro::image::ImageRectangle)),
                ui->imageWidget, SLOT(selectRectangle(QRect)));
	ui->imageWidget->setRectangleSelectionEnabled(true);
}

/**
 * \brief Destroy the images window
 */
imageswindow::~imageswindow() {
	delete ui;
}

/**
 * \brief accept the images proxy
 *
 * This also initially sets the list of images 
 */
void	imageswindow::setImages(snowstar::ImagesPrx images) {
	_images = images;
	if (!_images) {
		return;
	}

	// read images and display list of images
	snowstar::ImageList	list = _images->listImages();
	snowstar::ImageList::const_iterator	i;
	std::set<ImageInfo>	imageset;
	for (i = list.begin(); i != list.end(); i++) {
		std::string	name = *i;
		ImageInfo	info(name);
		info.age(_images->imageAge(name));
		info.size(_images->imageSize(name));
		imageset.insert(info);
	}

	// iterate through the set and add all items to the list
	ui->imageTree->blockSignals(true);
	std::set<ImageInfo>::const_iterator	j;
	for (j = imageset.begin(); j != imageset.end(); j++) {
		ImageInfo	info = *j;
		QStringList	list;
                list << QString(info.dateString().c_str());
                list << QString(info.timeString().c_str());
                list << QString::number(info.size());
                list << QString(info.name().c_str());
                QTreeWidgetItem *item = new QTreeWidgetItem(list,
                        QTreeWidgetItem::Type);
                ui->imageTree->addTopLevelItem(item);
	}
	ui->imageTree->blockSignals(false);
}

/**
 * \brief handle when the selected widget changes
 */
void	imageswindow::currentImageChanged(QTreeWidgetItem *current, QTreeWidgetItem * /* previous */) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "currentImageChanged");
	std::string	name(current->text(3).toLatin1());
	if (!_images) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "no _images");
		return;
	}
	snowstar::ImagePrx	image;
	try {
		image = _images->getImage(name);
	} catch (const std::exception& x) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot get image: %s", x.what());
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "got a new image");
	ui->imagedetailWidget->setImage(image);
}

void	imageswindow::setImage(astro::image::ImagePtr image) {
	ui->imageWidget->setImage(image);
}

void	imageswindow::deleteCurrentImage() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "delete image");

	QTreeWidgetItem	*current = ui->imageTree->currentItem();
	int	index = ui->imageTree->indexOfTopLevelItem(current);

	// get the name from it
	std::string	name(current->text(3).toLatin1().data());
	try {
		snowstar::ImagePrx	image;
		image = _images->getImage(name);
		image->remove();
	} catch (const std::exception& x) {
		return;
	}

	current = ui->imageTree->takeTopLevelItem(index);
	delete current;

	//astro::image::ImagePtr	imageptr;
	//ui->imageWidget->setImage(imageptr);
}

void	imageswindow::itemDoubleClicked(QTreeWidgetItem * /* item */, int /* column */) {
	ui->imagedetailWidget->loadImage();
}

void	imageswindow::closeEvent(QCloseEvent * /* event */) {
	deleteLater();
}

} // namespace snowgui
