/*
 * repositorywindow.h -- information about images in a repository
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_REPOSITORYWINDOW_H
#define SNOWGUI_REPOSITORYWINDOW_H

#include <QWidget>
#include <AstroDiscovery.h>
#include <repository.h>
#include <QTreeWidgetItem>

namespace snowgui {

namespace Ui {
	class repositorywindow;
}

class repositorywindow : public QWidget {
	Q_OBJECT

	std::string	_reponame;
	int	_imageid;

public:
	repositorywindow(QWidget *parent,
		astro::discover::ServiceObject serviceobject);
	~repositorywindow();

	void	setRepositories(snowstar::RepositoriesPrx);
private:
	Ui::repositorywindow *ui;
	astro::discover::ServiceObject	_serviceobject;
	snowstar::RepositoriesPrx	_repositories;

	void	addImages(QTreeWidgetItem *top, const std::string& reponame);

	ImagePtr	currentImage();

	void	deleteMulti(QList<QTreeWidgetItem*>&);
	void	saveMulti(QList<QTreeWidgetItem*>&);

public slots:
	void	openClicked();
	void	saveClicked();
	void	deleteClicked();
	void	currentImageChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void	itemDoubleClicked(QTreeWidgetItem *, int);

protected:
	void	closeEvent(QCloseEvent *);
};


} // namespace snowgui
#endif // SNOWGUI_REPOSITORYWINDOW_H
