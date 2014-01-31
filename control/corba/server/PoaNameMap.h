/*
 * PoaNameMap.h -- templates for POA name mapping
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#ifndef _PoaNameMap_h
#define _PoaNameMap_h

#include <OrbSingleton.h>
#include <module.hh>
#include <camera.hh>

namespace Astro {

template<typename device>
PoaName	poaname();

template<>
PoaName	poaname<Camera>();

template<>
PoaName	poaname<Ccd>();

template<>
PoaName	poaname<Cooler>();

template<>
PoaName	poaname<GuiderPort>();

template<>
PoaName	poaname<FilterWheel>();

template<>
PoaName	poaname<Focuser>();

template<>
PoaName	poaname<Guider>();

} // namespace Astro

#endif /* _PoaNameMap_h */
