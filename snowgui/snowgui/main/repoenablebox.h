/*
 * repoenablebox.h -- widget to configure hidden flag on repository
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef REPOENABLEBOX_H
#define REPOENABLEBOX_H

#include <QCheckBox>
#include <repository.h>

namespace snowgui {

namespace Ui {
	class repositoryconfigurationwidget;
}

/**
 * \brief A reusable GUI component to 
 */
class repoenablebox : public QCheckBox {
	Q_OBJECT

	std::string	_reponame;
	snowstar::RepositoriesPrx	_repositories;
public:
	explicit repoenablebox(QWidget *parent = NULL);
	~repoenablebox();

	const std::string&	reponame() const { return _reponame; }
	void	reponame(const std::string& r) { _reponame = r; }

	void	setRepositories(snowstar::RepositoriesPrx repositories);

public slots:
	void	enableToggled(bool);

};

} // namespace snowgui

#endif /* REPOENABLEBOX_H */
