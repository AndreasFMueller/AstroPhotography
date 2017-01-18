/*
 * taskconnectiondialog.cpp -- connection dialog for the task manager
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <taskconnectiondialog.h>
#include <taskmainwindow.h>

TaskConnectionDialog::TaskConnectionDialog(QWidget *parent)
	: ConnectionDialog(parent) {
}

TaskConnectionDialog::~TaskConnectionDialog() {
}

void	TaskConnectionDialog::accept() {
	ConnectionDialog::accept();

	TaskMainWindow	*taskmainwindow = new TaskMainWindow();
	taskmainwindow->show();
	taskmainwindow->setWindowTitle(servername);

	// close
	close();
}
