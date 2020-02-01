//
// repository.ice -- interface definition for repository replication
//
// (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <image.ice>

module snowstar {
	sequence<string>	uuidlist;

	/**
	 * \brief Information about an image an image repository
	 *
	 * This structure mirrors the ImageEnvelope class in the 
	 * the AstroProject.h header
	 */
	struct ImageInfo {
		int	id;
		string	uuid;
		string	filename;
		string	project;
		double	createdago;
		string	instrument;
		ImageSize	size;
		BinningMode	binning;
		double	exposuretime;
		double	temperature;
		string	purpose;
		double	observationago;
		string	filter;
		string	bayer;
		long	focus;
	};

	sequence<string>	projectnamelist;

	/**
	 * \brief Image Repository interface
	 *
	 * This interface does not give the full local access to a repository,
	 * as it is only intended to implement replication between a local
	 * and a remote repository.
	 */
	interface Repository extends Statistics {
		idlist		getIds();
		idlist		getIdsCondition(string condition);
		uuidlist	getUUIDs();
		uuidlist	getUUIDsCondition(string condition);
		projectnamelist	getProjectnames();
		bool		has(int id);
		bool		hasUUID(string uuid);
		int		getId(string uuid) throws NotFound;
		ImageBuffer	getImage(int id, ImageEncoding encoding)
					throws NotFound;
		ImageInfo	getInfo(int id) throws NotFound;
		int	save(ImageFile image) throws Exists;
		int	count();
		void	remove(int id) throws NotFound;
	};

	sequence<string>	reponamelist;

	struct RepositorySummary {
		string	name;
		string	directory;
		string	database;
		int	count;
		bool	hidden;
	};

	sequence<RepositorySummary>	reposummarylist;

	/**
	 * \brief Repository selection on server
	 *
	 * There may be multiple repositories on the server, and this method
	 * allows to retrieve a proxy to a repository.
	 */
	interface Repositories extends Statistics {
		reponamelist	list();
		reposummarylist	summarylist();
		bool	has(string reponame);
		Repository*	get(string reponame) throws NotFound;
		void	add(string reponame, string repodirectory)
				throws Exists, BadParameter;
		void	remove(string reponame, bool removecontents)
				throws NotFound, IOException;
		void	setHidden(string reponame, bool hidden)
				throws NotFound;
	};
};
