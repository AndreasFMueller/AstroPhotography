/*
 * connectiondialog.h -- dialog to set up CORBA and name server connection
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <omniORB4/CORBA.h>
#include <QDialog>

namespace Ui {
class ConnectionDialog;
}

/**
 * \brief Dialog to select a server connection
 *
 * This class prompts the user to select a server to connect to. It then
 * creates the connection and launches the initial application dialog.
 * For this, the accept slot should by overridden in derived classes
 */
class ConnectionDialog : public QDialog
{
	Q_OBJECT
public:
static	CORBA::ORB_ptr	orb;
static	CosNaming::NamingContext_var	namingcontext;
static	QString	servername;
private:
	void	buildconnection(const QString _servername);

public:
	explicit ConnectionDialog(QWidget *parent = 0);
	~ConnectionDialog();

public slots:
	void	accept();

private:
    Ui::ConnectionDialog *ui;
};

#endif // CONNECTIONDIALOG_H
