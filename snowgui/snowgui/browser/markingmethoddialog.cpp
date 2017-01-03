/*
 * markingmethoddialog.cpp
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include "markingmethoddialog.h"
#include "ui_markingmethoddialog.h"
#include <AstroDebug.h>

namespace snowgui {

/**
 * \brief Construct a marking method selection dialog
 */
markingmethoddialog::markingmethoddialog(QWidget *parent)
	: QDialog(parent), ui(new Ui::markingmethoddialog) {
	ui->setupUi(this);

	_method = MarkSubdirectory;
	_prefix = "bad-";
	_subdirectory = "bad";
	
	connect(ui->subdirectoryField, SIGNAL(editingFinished()),
		this, SLOT(subdirEditingFinished()));
	connect(ui->prefixField, SIGNAL(editingFinished()),
		this, SLOT(prefixEditingFinished()));
	connect(ui->subdirButton, SIGNAL(clicked(bool)),
		this, SLOT(subdirClicked(bool)));
	connect(ui->prefixButton, SIGNAL(clicked(bool)),
		this, SLOT(prefixClicked(bool)));
}

/**
 * \brief Destroy the marking method selection dialog
 */
markingmethoddialog::~markingmethoddialog() {
	delete ui;
}

void	markingmethoddialog::method(MarkingMethod m) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "method: %d", (int)m);
}

void	markingmethoddialog::subdirClicked(bool checked) {
	if (checked) {
		_method = MarkSubdirectory;
	}
}

void	markingmethoddialog::prefixClicked(bool checked) {
	if (checked) {
		_method = MarkPrefix;
	}
}

void	markingmethoddialog::subdirEditingFinished() {
	_subdirectory = std::string(ui->subdirectoryField->text().toLatin1().data());
}

void	markingmethoddialog::prefixEditingFinished() {
	_prefix = std::string(ui->prefixField->text().toLatin1().data());
}

} // namespace snowgui
