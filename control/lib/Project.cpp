/*
 * Project.cpp -- Project implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroProject.h>
#include <includes.h>

namespace astro {
namespace project {

Project::Project(const std::string& name) : _name(name) {
	_started = time(NULL);
}

PartPtr	Project::part(long partno) {
	std::map<long, PartPtr>::const_iterator	i = parts.find(partno);
	if (i == parts.end()) {
		throw std::runtime_error("part no not found");
	}
	return i->second;
}

void	Project::add(PartPtr part) {
	parts.insert(std::make_pair(part->partno(), part));
}

} // namespace project
} // namespace astro
