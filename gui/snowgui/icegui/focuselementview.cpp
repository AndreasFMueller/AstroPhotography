/*
 * focuselementview.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "focuselementview.h"
#include "ui_focuselementview.h"
#include <AstroDebug.h>
#include <QLabel>
#include <QPixmap>
#include <QScrollBar>

namespace snowgui {

FocusElementView::FocusElementView(QWidget *parent)
	: QWidget(parent), ui(new Ui::FocusElementView) {
	ui->setupUi(this);

	connect(ui->rawimageArea->horizontalScrollBar(),
		SIGNAL(valueChanged(int)),
		this,
		SLOT(sliderChanged(int)));
	connect(ui->rawimageArea->verticalScrollBar(),
		SIGNAL(valueChanged(int)),
		this,
		SLOT(sliderChanged(int)));
	connect(ui->evaluatedimageArea->horizontalScrollBar(),
		SIGNAL(valueChanged(int)),
		this,
		SLOT(sliderChanged(int)));
	connect(ui->evaluatedimageArea->verticalScrollBar(),
		SIGNAL(valueChanged(int)),
		this,
		SLOT(sliderChanged(int)));
}

FocusElementView::~FocusElementView() {
	delete ui;
}

void	FocusElementView::setFocusElement(snowstar::FocusElement element) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add focus element");
	_element = element;
	// convert image data into pixmaps
	QPixmap	*rawpixmap = new QPixmap();
	if (_element.raw.data.size() > 0) {
		rawpixmap->loadFromData(_element.raw.data.data(),
			_element.raw.data.size());
	}
	QLabel	*rawlabel = new QLabel();
	rawlabel->setPixmap(*rawpixmap);
	rawlabel->setFixedSize(rawpixmap->width(), rawpixmap->height());
	rawlabel->setMinimumSize(rawpixmap->width(), rawpixmap->height());
	ui->rawimageArea->setWidget(rawlabel);

	QPixmap	*evaluatedpixmap = new QPixmap();
	if (_element.evaluated.data.size() > 0) {
		evaluatedpixmap->loadFromData(_element.evaluated.data.data(),
			_element.evaluated.data.size());
	}
	QLabel	*evaluatedlabel = new QLabel();
	evaluatedlabel->setPixmap(*evaluatedpixmap);
	evaluatedlabel->setFixedSize(evaluatedpixmap->width(),
		evaluatedpixmap->height());
	evaluatedlabel->setMinimumSize(evaluatedpixmap->width(),
		evaluatedpixmap->height());
	ui->evaluatedimageArea->setWidget(evaluatedlabel);
}

void	FocusElementView::sliderChanged(int v) {
	if(sender() == ui->rawimageArea->horizontalScrollBar()) {
		ui->evaluatedimageArea->horizontalScrollBar()->setValue(v);
	}
	if(sender() == ui->rawimageArea->verticalScrollBar()) {
		ui->evaluatedimageArea->verticalScrollBar()->setValue(v);
	}
	if(sender() == ui->evaluatedimageArea->horizontalScrollBar()) {
		ui->rawimageArea->horizontalScrollBar()->setValue(v);
	}
	if(sender() == ui->evaluatedimageArea->verticalScrollBar()) {
		ui->rawimageArea->verticalScrollBar()->setValue(v);
	}
}

} // namespace snowgui
