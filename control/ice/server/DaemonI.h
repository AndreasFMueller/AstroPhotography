/*
 * DaemonI.h -- classes for the daemon
 *
 * (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _DaemonI_h
#define _DaemonI_h

#include <types.h>
#include <Server.h>

namespace snowstar {

class DaemonI : virtual public Daemon {
	Server&	_server;
public:
	DaemonI(Server& server) : _server(server) { }
	virtual ~DaemonI() { }
	void	reloadRepositories(const Ice::Current& current);
	void	shutdownServer(Ice::Float delay, const Ice::Current& current);
	void	shutdownSystem(Ice::Float delay, const Ice::Current& current);
	void	restartServer(Ice::Float delay, const Ice::Current& current);
	FileInfo	statFile(const std::string& filename,
					const Ice::Current& current);
	FileInfo	statDevice(const std::string& devicename,
					const Ice::Current& current);
	DirectoryInfo	statDirectory(const std::string& dirname,
					const Ice::Current& current);
	void	mount(const std::string& device, const std::string& mountpoint,
			const Ice::Current& current);
	void	unmount(const std::string& mountpoint,
			const Ice::Current& current);
	Ice::Long	getSystemTime(const Ice::Current& current);
	void	setSystemTime(Ice::Long unixtime, const Ice::Current& current);
	std::string	osVersion(const Ice::Current& current);
	std::string	astroVersion(const Ice::Current& current);
	std::string	snowstarVersion(const Ice::Current& current);
	Sysinfo	getSysinfo(const Ice::Current& current);
	float	daemonUptime(const Ice::Current& current);
	float	getTemperature(const Ice::Current& current);
	float	cputime(const Ice::Current& current);
};

} // namespace snowstar

#endif /* _DaemonI_h */
