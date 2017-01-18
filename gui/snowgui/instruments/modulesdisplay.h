/*
 * modulesdisplay.h
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_MODULESDISPLAY_H
#define SNOWGUI_MODULESDISPLAY_H

#include <QWidget>
#include <QListWidgetItem>
#include <device.h>

namespace snowgui {

namespace Ui {
	class modulesdisplay;
}

class modulesdisplay : public QWidget {
	Q_OBJECT

public:
	explicit modulesdisplay(QWidget *parent = 0);
	~modulesdisplay();

	void	setModules(snowstar::ModulesPrx);

	bool	deviceSelected();
	std::string	selectedDevicename();

public slots:
	void	moduleChanged(QString modulename);

private:
	Ui::modulesdisplay *ui;
	snowstar::ModulesPrx	_modules;

	QListWidgetItem *selectedItem();
	void	add(snowstar::DeviceLocatorPrx locator,
			enum snowstar::devicetype type);
};


} // namespace snowgui
#endif // SNOWGUI_MODULESDISPLAY_H
