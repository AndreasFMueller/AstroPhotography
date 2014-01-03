/*
 * Output.h -- output of some basic Astro types
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _Output_h
#define _Output_h

#include <types.hh>
#include <camera.hh>
#include <iostream>

namespace Astro {

std::ostream&	operator<<(std::ostream& out, const Astro::BinningMode& mode);
std::ostream&	operator<<(std::ostream& out, const Astro::ImagePoint& point);
std::ostream&	operator<<(std::ostream& out, const Astro::ImageSize& size);
std::ostream&   operator<<(std::ostream& out,
                        const Astro::ImageRectangle& rectangle); 

} // namespace Astro

#endif /* _Output_h */
