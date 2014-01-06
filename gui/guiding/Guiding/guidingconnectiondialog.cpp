/*
 * guidingconnectiondialog.cpp -- implementation of guiding connection start
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidingconnectiondialog.h>
#include <AstroDebug.h>
#include <guideropendialog.h>

GuidingConnectionDialog::GuidingConnectionDialog(QWidget *parent)
	: ConnectionDialog(parent) {
}

GuidingConnectionDialog::~GuidingConnectionDialog() {
}

void	GuidingConnectionDialog::accept() {
	// accept the way the connection dialog would
	ConnectionDialog::accept();

	GuiderOpenDialog	*guideropendialog = new GuiderOpenDialog();
	guideropendialog->show();

	// close
	close();
}

