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
		sequence<long>	getId(string uuid);
		ImageFile	getImage(long id);
		void	save(ImageFile imagefile);
		void	remove(long id);
	}

	/**
	 * \brief Repository selection on server
	 *
	 * There may be multiple repositories on the server, and this method
	 * allows to retrieve a proxy to a repository.
	 */
	interface Repositories {
		Repository	get(string reponame);
	}
};
