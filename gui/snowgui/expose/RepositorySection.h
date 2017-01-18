/*
 * RepositorySection.h -- Object describing the section of the repository
 *
 * (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _RepositorySection_h
#define _RepositorySection_h

#include <string>
#include <camera.h>

namespace snowgui {

/**
 * \brief Key to repository sections
 *
 * The key is just the purpose and the filter name
 */
class RepositoryKey {
	snowstar::ExposurePurpose	_purpose;
	std::string	_filtername;
public:
	snowstar::ExposurePurpose	purpose() const { return _purpose; }
	std::string	purposeString() const;
	const std::string&	filtername() const { return _filtername; }

	RepositoryKey(snowstar::ExposurePurpose purpose,
		const std::string& filtername);
	RepositoryKey(const std::string& purpose,
		const std::string& filtername);
	RepositoryKey(snowstar::ExposurePurpose purpose = snowstar::ExLIGHT);
	RepositoryKey(const std::string& purpose);
	RepositoryKey(const RepositoryKey& key);

	std::string	toString() const;

	bool	operator<(const RepositoryKey& key) const;
};

/**
 * \brief The actual repository seciton
 *
 * The section also contains the filter index and the index of the top level
 * widget in the tree
 */
class RepositorySection : public RepositoryKey {
	int	_filterindex;
	int	_index;
public:
	int	filterindex() const { return _filterindex; }
	int	index() const { return _index; }
	void	index(int i) { _index = i; }

	RepositorySection(snowstar::ExposurePurpose purpose,
		const std::string& filtername,
		int filterindex);
	RepositorySection(snowstar::ExposurePurpose purpose);
	RepositorySection(const RepositoryKey& key, int filterindex, int index);
	RepositorySection(const RepositoryKey& key, int index);

};

} // namespace snowgui

#endif /* _RepositorySection_h */

