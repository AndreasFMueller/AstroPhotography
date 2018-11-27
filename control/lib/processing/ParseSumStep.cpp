/*
 * ParseSumStep.cpp
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroProcess.h>
#include <ProcessorParser.h>

namespace astro {
namespace process {

/**
 * \brief start the stacking step
 */
void	ProcessorParser::startSum(const attr_t& attrs) {
	// create the stacking step
	SumStep	*ss = new SumStep(nodePaths());
	ProcessingStepPtr	sstep(ss);

	// remember everyhwere
	push(sstep);

	startCommon(attrs);

	if (ss->baseimage()) {
		sstep->add_precursor(ss->baseimage());
	}
}

} // namespace process
} // namespace astro
