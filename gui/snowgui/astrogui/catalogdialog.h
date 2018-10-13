/*
 * catalogdialog.h
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_CATALOGDIALOG_H
#define SNOWGUI_CATALOGDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <AstroCoordinates.h>
#include <AstroCatalog.h>

namespace snowgui {

namespace Ui {
	class CatalogDialog;
}

class CatalogDialog : public QDialog {
	Q_OBJECT
	astro::catalog::DeepSkyCatalogPtr	_catalog;

public:
	explicit CatalogDialog(QWidget *parent = 0);
	~CatalogDialog();

	void	closeEvent(QCloseEvent *);

private:
	Ui::CatalogDialog *ui;

	void	searchCommon(const std::string& name);

signals:
	void	objectSelected(astro::RaDec);

public slots:
	void	searchClicked();
	void	searchChanged(const QString&);
	void	textEdited(const QString&);
	void	nameActivated(QListWidgetItem *);
};

} // namespace snowgui
#endif // SNOWGUI_CATALOGDIALOG_H
