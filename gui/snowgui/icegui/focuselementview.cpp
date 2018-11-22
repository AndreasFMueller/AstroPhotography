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
#include <QMenu>

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
	// custom context menu
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this,
		SIGNAL(customContextMenuRequested(const QPoint &)),
                this,
		SLOT(showContextMenu(const QPoint &)));

	setShowRawimage(true);
	setShowEvaluatedimage(true);
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

void	FocusElementView::show_rawimage(bool r) {
	_show_rawimage = r;
	ui->rawimageArea->setHidden(!_show_rawimage);
}

void	FocusElementView::show_evaluatedimage(bool e) {
	_show_evaluatedimage = e;
	ui->evaluatedimageArea->setHidden(!_show_evaluatedimage);
}

void	FocusElementView::setShowRawimage(bool r) {
	show_rawimage(r);
	repaint();
}

void	FocusElementView::setShowEvaluatedimage(bool e) {
	show_evaluatedimage(e);
	repaint();
}

void	FocusElementView::toggleShowRawimage() {
	setShowRawimage(!show_rawimage());
	if ((!show_rawimage()) && (!show_evaluatedimage())) {
		setShowEvaluatedimage(true);
	}
}

void	FocusElementView::toggleShowEvaluatedimage() {
	setShowEvaluatedimage(!show_evaluatedimage());
	if ((!show_rawimage()) && (!show_evaluatedimage())) {
		setShowRawimage(true);
	}
}

void	FocusElementView::showContextMenu(const QPoint& point) {
	QMenu	contextMenu(QString("Display Options"), this);

	QAction	actionRaw(QString("raw image"), this);
	actionRaw.setCheckable(true);
	actionRaw.setChecked(show_rawimage());
	contextMenu.addAction(&actionRaw);
	connect(&actionRaw, SIGNAL(triggered()),
		this, SLOT(toggleShowRawimage()));

	QAction	actionEvaluated(QString("evaluated image"), this);
	actionEvaluated.setCheckable(true);
	actionEvaluated.setChecked(show_evaluatedimage());
	contextMenu.addAction(&actionEvaluated);
	connect(&actionEvaluated, SIGNAL(triggered()),
		this, SLOT(toggleShowEvaluatedimage()));

	contextMenu.exec(mapToGlobal(point));
}

} // namespace snowgui
