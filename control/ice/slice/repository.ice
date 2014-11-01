//
// repo.ice -- interface definition for repository replication
//
// (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
//
#include <image.ice>

module snowstar {
	sequence<string>	uuidlist;

	/**
	 * \brief Image Repository interface
	 *
	 * This interface does not give the full local access to a repository,
	 * as it is only intended to implement replication between a local
	 * and a remote repository.
	 */
	interface Repository {
		uuidlist	getUUIDs();
		int		getId(string uuid);
		ImageFile	getImage(int id);
		int	save(ImageFile image);
		void	remove(int id);
	};

	sequence<string>	reponamelist;

	/**
	 * \brief Repository selection on server
	 *
	 * There may be multiple repositories on the server, and this method
	 * allows to retrieve a proxy to a repository.
	 */
	interface Repositories {
		reponamelist	list();
		Repository*	get(string reponame);
	};
};
