/*
 * configurationwidget.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef CONFIGURATIONWIDGET_H
#define CONFIGURATIONWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include <AstroConfig.h>

namespace snowgui {

namespace Ui {
	class configurationwidget;
}

class configurationwidget : public QWidget {
	Q_OBJECT

public:
	explicit configurationwidget(QWidget *parent = nullptr);
	~configurationwidget();

	virtual	void	filltable();

	void	closeEvent(QCloseEvent *);

	astro::config::ConfigurationKey	key(int row);
	astro::config::ConfigurationKey	key(QTableWidgetItem *item);

	// interface to access configuration data
	virtual std::list<astro::config::ConfigurationKey>	listkeys();
	virtual bool	has(const astro::config::ConfigurationKey& key);
	virtual std::string	description(const astro::config::ConfigurationKey& key);
	virtual std::string	value(const astro::config::ConfigurationKey& key);
	virtual void	set(const astro::config::ConfigurationKey& key,
				const std::string& value);
	virtual void	remove(const astro::config::ConfigurationKey& key);

private:
	Ui::configurationwidget *ui;

public slots:
	void	refreshClicked();
	void	deleteClicked();
	void	valueChanged(int, int);
	void	selectionChanged();
};

} // namespace snowgui

#endif // CONFIGURATIONWIDGET_H
