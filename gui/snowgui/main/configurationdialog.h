/*
 * configurationdialog.h -- Dialog to configure the server
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_CONFIGURATIONDIALOG_H
#define SNOWGUI_CONFIGURATIONDIALOG_H

#include <QDialog>
#include <QTimer>
#include <types.h>
#include <device.h>
#include <AstroDiscovery.h>

namespace snowgui {

namespace Ui {
	class configurationdialog;
}

class timesourceinfo {
public:
	std::string			name;
	snowstar::DeviceLocatorPrx	locator;
	snowstar::MountPrx		mount;
};

typedef std::shared_ptr<timesourceinfo>	timesourceinfoPtr;

class configurationdialog : public QDialog {
	Q_OBJECT

	bool	getService(const std::string& name);
	void    changevalue(const std::string& name, bool defaultvalue,
			bool newvalue);
	bool	_servicechangewarning;
	bool	_mounting;

	std::vector<timesourceinfoPtr>	_timesources;

	QTimer	_statusTimer;
public:
	configurationdialog(QWidget *parent,
		astro::discover::ServiceObject serviceobject);
	~configurationdialog();

	void	setConfiguration(snowstar::ConfigurationPrx);
	void	setDaemon(snowstar::DaemonPrx);
	void	setModules(snowstar::ModulesPrx);

public slots:
	// services offered
	void	devicesToggled(bool);
	void	instrumentsToggled(bool);
	void	imagesToggled(bool);
	void	guidingToggled(bool);
	void	focusingToggled(bool);
	void	repositoriesToggled(bool);
	void	tasksToggled(bool);
	void	gatewayToggled(bool);
	void	restartClicked();

	// repository management
	void	repodbChanged(QString);
	void	repodbClicked();

	// mountpoint management
	void	deviceChanged(QString);
	void	mountpointChanged(QString);
	void	mountClicked();

	// time management
	void	syncClicked();
	void	setfromsourceClicked();
	void	timeUpdate();
	void	timesourceSelected(int);

	// shutdown
	void	shutdownClicked();
	void	systemClicked();

	void	closeEvent(QCloseEvent *);
private:
	Ui::configurationdialog *ui;
	astro::discover::ServiceObject	_serviceobject;
	snowstar::ConfigurationPrx	_configuration;
	snowstar::DaemonPrx	_daemon;
	snowstar::ModulesPrx	_modules;
	snowstar::MountPrx	_mount;

	void	operationFailed(const std::string& s);
};


} // namespace snowgui
#endif // SNOWGUI_CONFIGURATIONDIALOG_H
