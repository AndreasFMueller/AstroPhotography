/*
 * cli.h. -- embeddable command line interpreter
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _astrocli_h
#define _astrocli_h 

#include <variables.h>

namespace astro {
namespace cli {

class cli {
public:
	variables	vars;
	int	parse(const char *filename);
	friend std::ostream&	operator<<(std::ostream& out, const cli& c);
};

std::ostream&	operator<<(std::ostream& out, const cli& c);

class sharedcli {
	static cli	*c;
public:
	sharedcli();
	sharedcli(cli *_c);
	int	parse(const char *filename);
	friend std::ostream&	operator<<(std::ostream& out, const sharedcli& c);
	variables&	vars() { return c->vars; }
};

std::ostream&	operator<<(std::ostream& out, const sharedcli& c);

} // namespace cli
} // namespace astro

#endif /* _astrocli_h */
