/*
 * focuselementview.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "focuselementview.h"
#include "ui_focuselementview.h"
#include <AstroDebug.h>

namespace snowgui {

FocusElementView::FocusElementView(QWidget *parent)
	: QWidget(parent), ui(new Ui::FocusElementView) {
	ui->setupUi(this);
}

FocusElementView::~FocusElementView() {
	delete ui;
}

void	FocusElementView::setFocusElement(snowstar::FocusElement element) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "add focus element");
	_element = element;
}

} // namespace snowgui
