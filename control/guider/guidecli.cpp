/*
 * guidecli.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <guidecli.h>
#include <stdexcept>

namespace astro {
namespace cli {

guidecli::guidecli(commandfactory& cf) : cli(cf) {
}

guidecli::~guidecli() {
}

guidesharedcli::guidesharedcli() : sharedcli() {
}

guidesharedcli::guidesharedcli(cli *_c) : sharedcli(_c) {
	if (NULL == dynamic_cast<guidecli*>(_c)) {
		throw std::runtime_error("not a guidecli pointer");
	}
}

guidecli	*guidesharedcli::operator->() {
	guidecli	*g = dynamic_cast<guidecli*>(c);
	if (NULL == g) {
		throw std::runtime_error("not a guidecli pointer");
	}
	return g;
}

} // namespace cli
} // namespace astro
