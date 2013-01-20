/*
 * AstroIO.h -- classes and functions to perform image IO to/from IO
 *                files
 *
 * (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
 * $Id$
 */
#ifndef _AstroIO_h
#define _AstroIO_h

namespace astro {

class FITSfile {
	std::string	filename;
public:
};

class FITSinfile : public FITSfile {
};

class FITSoutfile : public FITSfile {
};


} // namespace astro

#endif /* _AstroIO_h */
