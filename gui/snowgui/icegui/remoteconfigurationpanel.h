/*
 * remoteconfigurationpanel.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_REMOTECONFIGURATIONPANEL_H
#define SNOWGUI_REMOTECONFIGURATIONPANEL_H

#include <QWidget>
#include <AstroDiscovery.h>

namespace snowgui {

namespace Ui {
	class remoteconfigurationpanel;
}

class remoteconfigurationpanel : public QWidget {
	Q_OBJECT

public:
	explicit remoteconfigurationpanel(QWidget *parent = nullptr);
	~remoteconfigurationpanel();

	void	setServiceObject(astro::discover::ServiceObjectPtr serviceobject);

private:
	Ui::remoteconfigurationpanel *ui;
};

} // namespace snowgui

#endif // SNOWGUI_REMOTECONFIGURATIONPANEL_H
