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

	void	download(Astro::TaskQueue_var taskqueue,
			const std::list<long>& taskids);
private:
	void	download(Astro::TaskQueue_var taskqueue, long taskid);
	bool	usetaskid() const;
	std::string	filename(long taskid);
	std::string	filename(const Astro::TaskInfo_var& info,
				const Astro::TaskParameters_var& parameters);
};

#endif /* _downloadparameters */
