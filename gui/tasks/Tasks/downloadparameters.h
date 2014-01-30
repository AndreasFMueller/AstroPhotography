/*
 * downloadparameters.h -- parameters used for file download
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _downloadparameters_h
#define _downloadparameters_h

#include <QString>
#include <list>
#include <tasks.hh>

class fileinfo {
public:
	long	size;
	std::string	name;
	fileinfo(const std::string& _name) : name(_name) { }
};

/**
 * \brief Download Parameters clas
 *
 * When FITS files are downloaded from the server, names have to be 
 * created. The name encodes various important parameters used during
 * the exposure, and the instance variables of theis method track which
 * parameters need to be included. It also contains some methods to
 * produce a filename from from the task information and finally to
 * perform the download.
 */
class DownloadParameters {
public:
	QString	directory;
	QString	prefix;
	bool	exposuretime;
	bool	binning;
	bool	shutter;
	bool	filter;
	bool	temperature;
	bool	date;

	DownloadParameters();

	std::list<fileinfo>	download(Astro::TaskQueue_var& taskqueue,
			const std::list<long>& taskids);
	std::string	toString() const;
private:
	fileinfo	download(Astro::TaskQueue_var& taskqueue, long taskid);
	std::string	filename(const Astro::TaskInfo_var& info,
				const Astro::TaskParameters_var& parameters);
};

std::ostream&	operator<<(std::ostream& out, const DownloadParameters& p);

#endif /* _downloadparameters */
