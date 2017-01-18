/*
 * guidingconnectiondialog.h -- connection dialog that starts guiding
 *                              mainwindow
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _guidingconnectiondialog_h
#define _guidingconnectiondialog_h

#include <connectiondialog.h>

class GuidingConnectionDialog : public ConnectionDialog
{
	Q_OBJECT

private:

	void	buildconnection(const QString servername);

public:
    explicit GuidingConnectionDialog(QWidget *parent = 0);
    ~GuidingConnectionDialog();

public slots:
	void	accept();

};

#endif /* _guidingconnectiondialog_h */
