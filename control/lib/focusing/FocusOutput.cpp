/*
 * FocusOutput.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroFocus.h>
#include <AstroDebug.h>

namespace astro {
namespace focusing {

/**
 * \brief Construct a Focus output given method and solver
 */
FocusOutput::FocusOutput(const std::string& method, const std::string& solver)
	: FocusInputBase(method, solver) {
}

/**
 * \brief Construct an Output object from base input 
 *
 * This essentially copies the parameters from the input without creating
 * any focus elements.
 *
 * \param input		The input to use for the parameters
 */
FocusOutput::FocusOutput(const FocusInputBase& input) : FocusInputBase(input) {
}

/**
 * \brief Construct Output from Image file names
 *
 * This method constructs a FocusOutput object containing all the image 
 * names without processing the images.
 *
 * \param input		FocusInput object
 */
FocusOutput::FocusOutput(const FocusInput& input) : FocusInputBase(input) {
	// copy the file names
	std::for_each(input.begin(), input.end(),
		[&](const std::pair<unsigned long, std::string>& p) {
			FocusElement	fe(p.first);
			fe.filename = p.second;
			insert(std::make_pair(fe.pos(), fe));
		}
	);
}

/**
 * \brief Construct Output from Image file names
 *
 * This method constructs a FocusOutput object containing all the images
 * without processing the images.
 *
 * \param input		FocusInputImages object
 */
FocusOutput::FocusOutput(const FocusInputImages& input)
	: FocusInputBase(input) {
	std::for_each(input.begin(), input.end(),
		[&](const std::pair<unsigned long, ImagePtr>& p) {
			FocusElement	fe(p.first);
			fe.raw_image = p.second;
			insert(std::make_pair(fe.pos(), fe));
		}
	);
}

/**
 * \brief Get the focus items that will give the focus solution
 *
 * FocusItems is a set of positions and evaluation results.
 * A solver then takes this set as input to produce e solution for
 * the focusing result.
 */
FocusItems	FocusOutput::items() const {
	FocusItems	result;
	std::for_each(begin(), end(),
		[&](const std::pair<unsigned long, FocusElement>& p) {
			FocusItem	fi(p.first, p.second.value);
			result.insert(fi);
		}
	);
	return result;
}

} // namespace focusing
} // astro
