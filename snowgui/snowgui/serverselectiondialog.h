/*
 * serverselectiondialog.h -- dialog to select a service discovered via
 *                            ZeroConf
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SERVERSELECTIONDIALOG_H
#define SERVERSELECTIONDIALOG_H

#include <QDialog>
#include <AstroDiscovery.h>

namespace Ui {
class ServerSelectionDialog;
}

class ServerSelectionDialog : public QDialog {
	Q_OBJECT
	astro::discover::ServiceDiscoveryPtr	_servicediscovery;

public:
	explicit ServerSelectionDialog(QWidget *parent,
		astro::discover::ServiceDiscoveryPtr servicediscovery);
	~ServerSelectionDialog();

public slots:
	void	accept();

private:
	Ui::ServerSelectionDialog *ui;
};

#endif // SERVERSELECTIONDIALOG_H
