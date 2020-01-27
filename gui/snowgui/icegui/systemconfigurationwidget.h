/*
 * systemconfigurationwidget.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_SYSTEMCONFIGURATIONWIDGET_H
#define SNOWGUI_SYSTEMCONFIGURATIONWIDGET_H

#include <QDialog>
#include <AstroDiscovery.h>

namespace snowgui {

namespace Ui {
	class SystemConfigurationWidget;
}

/**
 * \brief Class to access multiple configuration databases
 */
class SystemConfigurationWidget : public QDialog {
	Q_OBJECT

public:
	explicit SystemConfigurationWidget(QWidget *parent = nullptr);
	~SystemConfigurationWidget();

	void	setServiceObject(
			astro::discover::ServiceObjectPtr serviceobject);

private:
	Ui::SystemConfigurationWidget *ui;

public slots:
	void	closeEvent(QCloseEvent *);
};

} // namespace snowgui

#endif // SNOWGUI_SYSTEMCONFIGURATIONWIDGET_H
