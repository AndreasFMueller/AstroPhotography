/*
 * configurationdialog.h -- Dialog to configure the server
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_CONFIGURATIONDIALOG_H
#define SNOWGUI_CONFIGURATIONDIALOG_H

#include <QDialog>
#include <types.h>
#include <AstroDiscovery.h>

namespace snowgui {

namespace Ui {
	class configurationdialog;
}

class configurationdialog : public QDialog {
	Q_OBJECT

	bool	getService(const std::string& name);
	void    changevalue(const std::string& name, bool defaultvalue,
			bool newvalue);
	bool	_servicechangewarning;
	bool	_mounting;

public:
	configurationdialog(QWidget *parent,
		astro::discover::ServiceObject serviceobject);
	~configurationdialog();

	void	setConfiguration(snowstar::ConfigurationPrx);
	void	setDaemon(snowstar::DaemonPrx);


public slots:
	void	devicesToggled(bool);
	void	instrumentsToggled(bool);
	void	imagesToggled(bool);
	void	guidingToggled(bool);
	void	focusingToggled(bool);
	void	repositoriesToggled(bool);
	void	tasksToggled(bool);
	void	restartClicked();
	void	repodbChanged(QString);
	void	repodbClicked();

	void	deviceChanged(QString);
	void	mountpointChanged(QString);
	void	mountClicked();
private:
	Ui::configurationdialog *ui;
	astro::discover::ServiceObject	_serviceobject;
	snowstar::ConfigurationPrx	_configuration;
	snowstar::DaemonPrx	_daemon;

	void	operationFailed(const std::string& s);
};


} // namespace snowgui
#endif // SNOWGUI_CONFIGURATIONDIALOG_H
