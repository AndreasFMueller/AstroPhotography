/*
 * repositoryconfiguraitonwidget.h -- widget to configure repositories
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef REPOSITORYCONTROLLERWIDGET_H
#define REPOSITORYCONTROLLERWIDGET_H

#include <QWidget>
#include <repository.h>

namespace snowgui {

namespace Ui {
	class repositoryconfigurationwidget;
}

/**
 * \brief A reusable GUI component to 
 */
class repositoryconfigurationwidget : public QWidget {
	Q_OBJECT

	snowstar::RepositoriesPrx	_repositories;
public:
	explicit repositoryconfigurationwidget(QWidget *parent = NULL);
	~repositoryconfigurationwidget();

	void	setRepositories(snowstar::RepositoriesPrx repositories);

	void	readRepositories();

public slots:
	void	createClicked();

private:
	Ui::repositoryconfigurationwidget	*ui;

};

} // namespace snowgui

#endif /* REPOSITORYCONTROLLERWIDGET_H */
