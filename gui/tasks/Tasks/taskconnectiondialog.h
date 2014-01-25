/*
 * taskconnectiondialog.h -- connection dialog that starts task manager main
 *                           window
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _taskconnectiondialog_h
#define _taskconnectiondialog_h

#include <connectiondialog.h>

class TaskConnectionDialog : public ConnectionDialog {
	Q_OBJECT
private:
//	void	buildconnection(const QString servername);

public:
	explicit TaskConnectionDialog(QWidget *parent = 0);
	~TaskConnectionDialog();

public slots:
	void	accept();
};

#endif /* _taskconnectiondialog_h */
