/*
 * guidecli.h -- The derived cli class for the 
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _guidecli_h
#define _guidecli_h

#include <cli.h>
#include <module.hh>
#include <image.hh>
#include <tasks.hh>

namespace astro {
namespace cli {

class guidecli : public cli {
public:
	guidecli(commandfactory& cf);
	virtual ~guidecli();
	Astro::Modules_var	modules;
	Astro::Images_var	images;
	Astro::TaskQueue_var	taskqueue;
};


class guidesharedcli : public sharedcli {
public:
	guidesharedcli();
	guidesharedcli(cli *c);
	guidecli	*operator->();
};

} // namespace cli
} // namespace astro

#endif /* _guidecli_h */
