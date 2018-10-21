/*
 * exposewidget.h -- widget to control exposures
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_EXPOSEWIDGET_H
#define SNOWGUI_EXPOSEWIDGET_H

#include <InstrumentWidget.h>
#include <repository.h>
#include <RepositorySection.h>
#include <QTreeWidgetItem>

namespace snowgui {

namespace Ui {
	class exposewidget;
}

/**
 * \brief Expose widget
 *
 * The expose widget displays a list of images taken for a project
 */
class exposewidget : public InstrumentWidget {
	Q_OBJECT

public:
	explicit exposewidget(QWidget *parent = 0);
	~exposewidget();
	virtual void	instrumentSetup(
		astro::discover::ServiceObject serviceobject,
		snowstar::RemoteInstrument instrument);
	virtual void	setupComplete();
	void	setRepositories(snowstar::RepositoriesPrx repositories);

private:
	Ui::exposewidget *ui;
	snowstar::RepositoriesPrx	_repositories;
	std::string	_repositoryname;
	snowstar::RepositoryPrx	_repository;
	std::string	_projectname;
	snowstar::FilterWheelPrx	_filterwheel;
	snowstar::FocuserPrx		_focuser;

	std::map<RepositoryKey, int>	_repository_index;
	std::vector<RepositorySection>	_repository_sections;
	int	_imageid;
	QTreeWidgetItem	*_imageitem;
	int	_selectedfiles;

	void	updateHeaderlist();
	void	updateImagelist();

	astro::image::ImagePtr	currentImage();

signals:
	void	startExposure();
	void	offerImage(astro::image::ImagePtr, std::string);

public slots:
	void	repositoryChanged(const QString&);
	void	startClicked();
	void	projectChanged(const QString&);
	void	filterwheelSelected(snowstar::FilterWheelPrx);
	void	focuserSelected(snowstar::FocuserPrx);

	void	currentImageChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void	itemDoubleClicked(QTreeWidgetItem *, int);

	void	deleteClicked();
	void	saveClicked();
	void	openClicked();
	void	downloadClicked();

	void	imageproxyReceived(snowstar::ImagePrx);
};


} // namespace snowgui
#endif // SNOWGUI_EXPOSEWIDGET_H
