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
	void    changevalue(const std::string& name, bool defaultvalue, bool newvalue);
	bool	_servicechangewarning;

public:
	configurationdialog(QWidget *parent,
		astro::discover::ServiceObject serviceobject);
	~configurationdialog();

	void	setConfiguration(snowstar::ConfigurationPrx);

public slots:
	void	devicesToggled(bool);
	void	instrumentsToggled(bool);
	void	imagesToggled(bool);
	void	guidingToggled(bool);
	void	focusingToggled(bool);
	void	repositoriesToggled(bool);
	void	tasksToggled(bool);
	void	restartClicked();

private:
	Ui::configurationdialog *ui;
	astro::discover::ServiceObject	_serviceobject;
	snowstar::ConfigurationPrx	_configuration;
};


} // namespace snowgui
#endif // SNOWGUI_CONFIGURATIONDIALOG_H
