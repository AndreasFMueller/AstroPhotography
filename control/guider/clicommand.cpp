/*
 * clicommand.cpp -- factory for CLI commands
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil 
 */
#include <clicommand.h>
#include <helpcommand.h>
#include <listcommand.h>
#include <modulecommand.h>
#include <locatorcommand.h>
#include <helpcommand.h>
#include <cameracommand.h>
#include <filterwheelcommand.h>
#include <ccdcommand.h>
#include <coolercommand.h>
#include <sleepcommand.h>
#include <focusercommand.h>
#include <guiderfactorycommand.h>
#include <guidercommand.h>
#include <imagecommand.h>
#include <sstream>
#include <algorithm>

namespace astro {
namespace cli {


//////////////////////////////////////////////////////////////////////
// commandkey implementation
//////////////////////////////////////////////////////////////////////

commandkey::commandkey(const std::string& commandname)
	: std::pair<std::string, std::string>(commandname, std::string("")) {
}

commandkey::commandkey(const std::string& commandname,
	const std::string& subcommandname)
	: std::pair<std::string, std::string>(commandname, subcommandname) {
}

bool	commandkey::operator==(const commandkey& other) const {
	return (first == other.first) && (second == other.second);
}

bool	commandkey::operator<(const commandkey& other) const {
	if (first < other.first) {
		return true;
	}
	if (first == other.first) {
		return second < other.second;
	}
	return false;
}

std::string	commandkey::toString() const {
	if (second.size() > 0) {
		return first + " " + second;
	}
	return first;
}

std::ostream&	operator<<(std::ostream& out, const commandkey& key) {
	return out << key.toString();
}

//////////////////////////////////////////////////////////////////////
// auxiliary class to display command summaries
//////////////////////////////////////////////////////////////////////
class	commanddisplay {
	commandfactory&	_factory;
	std::ostream&	_out;
public:
	commanddisplay(commandfactory& factory, std::ostream& out)
		: _factory(factory), _out(out) { }
	void	operator()(const std::pair<commandkey,
			commandcreatorptr>& i);
};

void	commanddisplay::operator()(const std::pair<commandkey,
			commandcreatorptr>& i) {
	_out << i.first << "\t";
	_out << i.second->get(_factory)->summary();
	_out << std::endl;
}

//////////////////////////////////////////////////////////////////////
// commandfactory implementation
//////////////////////////////////////////////////////////////////////

commandfactory::commandfactory() {
	commandmap.insert(std::make_pair(
		commandkey("help"),
		commandcreatorptr(new commandcreator<helpcommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("list"),
		commandcreatorptr(new commandcreator<listcommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("locator"),
		commandcreatorptr(new commandcreator<locatorcommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("module"),
		commandcreatorptr(new commandcreator<modulecommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("focuser"),
		commandcreatorptr(new commandcreator<focusercommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("camera"),
		commandcreatorptr(new commandcreator<cameracommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("filterwheel"),
		commandcreatorptr(new commandcreator<filterwheelcommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("ccd"),
		commandcreatorptr(new commandcreator<ccdcommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("cooler"),
		commandcreatorptr(new commandcreator<coolercommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("guiderfactory"),
		commandcreatorptr(new commandcreator<guiderfactorycommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("guider"),
		commandcreatorptr(new commandcreator<guidercommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("image"),
		commandcreatorptr(new commandcreator<imagecommand>())
	));
	commandmap.insert(std::make_pair(
		commandkey("sleep"),
		commandcreatorptr(new commandcreator<sleepcommand>())
	));
}

commandcreatorptr	commandfactory::getcreator(const std::string& name,
	const std::vector<std::string>& arguments) {
	std::map<commandkey, commandcreatorptr>::iterator	i;
	if (arguments.size() > 0) {
		i = commandmap.find(commandkey(name, arguments[0]));
		if (i != commandmap.end()) {
			return i->second;
		}
	}
	i = commandmap.find(commandkey(name));
	if (i != commandmap.end()) {
		return i->second;
	}
	return commandcreatorptr();
}

clicommandptr	commandfactory::get(const std::string& name,
	const std::vector<std::string>& arguments) {
	commandcreatorptr	cp = getcreator(name, arguments);
	if (cp) {
		return cp->get(*this);
	}
	return clicommandptr();
}

std::string	commandfactory::summary() {
	std::ostringstream	out;
	std::for_each(commandmap.begin(), commandmap.end(),
		commanddisplay(*this, out));
	return out.str();
}

std::string	commandfactory::help(const std::string& name,
			const std::vector<std::string>& arguments) {
	clicommandptr	cp = get(name, arguments);
	if (cp) {
		return cp->help();
	}
	return std::string("command '") + name + std::string("' unknown\n");
}

} // namespace cli
} // namespace astro
