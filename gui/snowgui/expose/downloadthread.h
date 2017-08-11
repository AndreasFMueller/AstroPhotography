/*
 * downloadthread.h -- thread to download the files of a project
 *
 * (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef SNOWGUI_DOWNLOADTHREAD_H
#define SNOWGUI_DOWNLOADTHREAD_H

#include <QThread>
#include <QString>
#include <repository.h>
#include <list>

namespace snowgui {

/**
 * \brief individual items to download
 */
class downloaditem : public QObject {
	Q_OBJECT

	int	_imageid;
public:
	QString	_reponame;
	QString	_targetdirectory;
	QString	_targetfile;

	int	imageid() const { return _imageid; }

	std::string 	reponame() const {
		return std::string(_reponame.toLatin1().data());
	}
	std::string	targetdirectory() const {
		return std::string(_targetdirectory.toLatin1().data());
	}
	std::string	targetfile() const {
		return std::string(_targetfile.toLatin1().data());
	}

	downloaditem(const QString& reponame, int imageid,
		const QString& targetdirectory, const QString& targetfile)
		: _imageid(imageid), _reponame(reponame),
		  _targetdirectory(targetdirectory), _targetfile(targetfile) {
	}

	downloaditem(const std::string& reponame, int imageid,
		const std::string& targetdirectory,
		const std::string& targetfile)
		: _imageid(imageid),
		  _reponame(reponame.c_str()),
		  _targetdirectory(targetdirectory.c_str()),
		  _targetfile(targetfile.c_str()) {
	}

	downloaditem() : _imageid(-1) { }

	downloaditem(const downloaditem& other)
		: QObject(),
		  _imageid(other._imageid), _reponame(other._reponame),
		  _targetdirectory(other._targetdirectory),
		  _targetfile(other._targetfile) {
	}

	downloaditem&	operator=(const downloaditem& other) {
		_imageid = other._imageid;
		_reponame = other._reponame;
		_targetdirectory = other._targetdirectory;
		_targetfile = other._targetfile;
		return *this;
	}
};

typedef std::list<downloaditem>	downloadlist;

/**
 * \brief thread to perform the download of images of a project
 *
 * Downloading images can take a long time, especially over a slow link or
 * with a slow server. This thread moves the work to a separate thread so
 * that the main thread can stay responsive.
 */
class downloadthread : public QThread {
	Q_OBJECT
public:
	downloadthread(QObject *parent = 0);
	~downloadthread();

	void	set(snowstar::RepositoriesPrx repositories,
			const downloadlist& filelist);

signals:
	void	sendStatus(downloaditem);
	void	downloadComplete();
	void	downloadAborted();
public slots:
	void	stopProcess();
protected:
	void	run();
private:
	std::string	_directory;
	snowstar::RepositoriesPrx	_repositories;
	bool	_stopProcess;
	std::string	_errormsg;
	downloadlist	_filelist;
public:
	const std::string	errormsg() const { return _errormsg; }
};

} // namespace showgui

#endif /* SNOWGUI_DOWNLOADTHREAD_H */
