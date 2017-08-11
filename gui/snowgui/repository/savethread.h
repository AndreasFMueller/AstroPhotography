/*
 * savethread.h -- thread class to perform downloads of images
 *
 * (c) 2016 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_SAVETHREAD_H
#define SNOWGUI_SAVETHREAD_H

#include <QThread>
#include <repository.h>

namespace snowgui {

/**
 * \brief Class used for status updates
 */
class downloadstatus : public QObject {
	Q_OBJECT
public:
	QString	_reponame;
	int	_imageid;
	downloadstatus(const QString& reponame, int imageid)
		: _reponame(reponame), _imageid(imageid) {
	}
	downloadstatus() {
		_imageid = -1;
	}
	~downloadstatus() {
	}
	downloadstatus(const downloadstatus& other)
		: QObject(), _reponame(other._reponame), _imageid(other._imageid) {
	}
};

/**
 * \brief 
 */
class savethread : public QThread {
	Q_OBJECT
public:
	savethread(QObject *parent = 0);
	~savethread();
	void	set(const std::string& directory,
			snowstar::RepositoriesPrx repositories,
			const std::list<std::pair<std::string, int> >& images);
	const std::string&	errormsg() const;
signals:
	void	sendStatus(downloadstatus);
	void	downloadComplete();
	void	downloadAborted();
public slots:
	void	stopProcess();
protected:
	void	run();
private:
	std::string	_directory;
	snowstar::RepositoriesPrx	_repositories;
	std::list<std::pair<std::string, int> >	_images;
	bool	_stopProcess;
	std::string	_errormsg;
};

} // namespace snowgui

#endif /* SNOWGUI_SAVETHREAD_H */
