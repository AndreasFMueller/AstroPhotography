/*
 * cli.h. -- embeddable command line interpreter
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _astrocli_h
#define _astrocli_h 

#include <iostream>
#include <string>

namespace astro {
namespace cli {

class cli {
public:
	cli() { }
	virtual ~cli() { }
	int	parse(const char *filename);
	int	parse(std::istream *infile);
	std::string	toString() const;
	friend std::ostream&	operator<<(std::ostream& out, const cli& c);
};

std::ostream&	operator<<(std::ostream& out, const cli& c);

class sharedcli {
protected:
	static cli	*c;
public:
	sharedcli();
	sharedcli(cli *_c);
	int	parse(const char *filename);
	int	parse(std::istream *infile);
	std::string	toString() const;
	friend std::ostream&	operator<<(std::ostream& out,
					const sharedcli& c);
};

std::ostream&	operator<<(std::ostream& out, const sharedcli& c);

} // namespace cli
} // namespace astro

#endif /* _astrocli_h */
