/*
 * test2.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>

using namespace astro::usb;

int	main(int argc, char *argv[]) {
	Context	context;
	context.setDebugLevel(3);
	std::vector<DevicePtr>  devicelist = context.devices();
	int	ndevices = devicelist.size();
        std::vector<DevicePtr>::const_iterator  i;
        for (i = devicelist.begin(); i != devicelist.end(); i++) {
		std::cout << "Device on " << **i << std::endl;
//		DeviceDescriptorPtr     dd = (*i)->descriptor();
//		std::cout << *dd << std::endl;
		ConfigurationPtr        c = (*i)->activeConfig();
		std::cout << *c;
        }
}
