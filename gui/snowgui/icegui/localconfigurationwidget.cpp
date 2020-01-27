/**
 * \brief localconfigurationwidget.cpp
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "localconfigurationwidget.h"
#include "ui_localconfigurationwidget.h"

#include <version.h>
#include <sys/utsname.h>

namespace snowgui {

/**
 * \brief construct a localconfigurationwidget
 */
localconfigurationwidget::localconfigurationwidget(QWidget *parent)
	: QWidget(parent), ui(new Ui::localconfigurationwidget) {
	ui->setupUi(this);

	// get local system information
	struct utsname  u;
	uname(&u);
	ui->localsystemField->setText(QString(u.version));
	ui->localastroversionField->setText(QString(astro::version().c_str()));

	// make sure the table is filled
	ui->configurationWidget->filltable();
}

/**
 * \brief Destroy the localconfigurationwidget
 */
localconfigurationwidget::~localconfigurationwidget() {
	delete ui;
}

} // namespace snowgui
