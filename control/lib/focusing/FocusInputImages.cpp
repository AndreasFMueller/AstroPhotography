/*
 * FocusInputImages.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroIO.h>
#include <algorithm>

namespace astro {
namespace focusing {

/**
 * \brief Construct input with all the images open
 *
 * \param input		focus input with file names to open
 */
FocusInputImages::FocusInputImages(const FocusInput& input)
	: FocusInputBase(input) {
	// open all the images
	std::for_each(input.begin(), input.end(),
		[&](const std::pair<unsigned long, std::string>& p) {
			ImagePtr	image = input.image(p.first);
			insert(std::make_pair(p.first, image));
		}
	);
}

} // namespace focusing
} // namespace astro
