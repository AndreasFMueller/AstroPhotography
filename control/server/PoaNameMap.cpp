/*
 * PoaNameMap.cpp -- templates for POA name mapping
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <PoaNameMap.h>

namespace Astro {

template<>
PoaName	poaname<Camera>() {
	return PoaName::cameras();
}

template<>
PoaName	poaname<Ccd>() {
	return PoaName::cameras();
}

template<>
PoaName	poaname<Cooler>() {
	return PoaName::coolers();
}

template<>
PoaName	poaname<GuiderPort>() {
	return PoaName::guiderports();
}

template<>
PoaName	poaname<FilterWheel>() {
	return PoaName::filterwheels();
}

template<>
PoaName	poaname<Focuser>() {
	return PoaName::focusers();
}

} // namespace Astro
