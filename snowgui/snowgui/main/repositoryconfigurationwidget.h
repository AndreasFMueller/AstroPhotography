/*
 * repositoryconfiguraitonwidget.h -- widget to configure repositories
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef REPOSITORYCONTROLLERWIDGET_H
#define REPOSITORYCONTROLLERWIDGET_H

namespace snowgui {

namespace Ui {
	class repositorycontrollerwidget;
}

/**
 * \brief A reusable GUI component to 
 */
class repositorycontrollerwidget : public QWidget {
	Q_OBJECT

	snowstar::RepositoriesPrx	_repositories;
public:
	explicit repositorycontrollerwidget(QWidget *parent = NULL);
	~repositorycontrollerwidget();
};

} // namespace snowgui

#endif /* REPOSITORYCONTROLLERWIDGET_H */
