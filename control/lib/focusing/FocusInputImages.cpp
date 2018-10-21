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
 * \param input	focus input with file names to open
 */
FocusInputImages::FocusInputImages(const FocusInput& input)
	: FocusInputBase(input) {
	// open all the images
	FocusInputImages	*fi = this;
	std::for_each(input.begin(), input.end(),
		[fi](const std::pair<unsigned long, std::string>& p) mutable {
			io::FITSin	in(p.second);
			ImagePtr	image = in.read();
			fi->insert(std::make_pair(p.first, image));
		}
	);
}

} // namespace focusing
} // namespace astro
