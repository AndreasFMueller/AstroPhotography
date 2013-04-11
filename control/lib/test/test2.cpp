/*
 * test2.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <stdio.h>
#include <stdlib.h>

using namespace astro::usb;

int	main(int argc, char *argv[]) {
	Context	context;
	context.setDebugLevel(3);
	std::vector<DevicePtr>  devicelist = context.devices();
	int	ndevices = devicelist.size();
	std::cout << "number of devices: " << ndevices << std::endl;
        std::vector<DevicePtr>::const_iterator  i;
	int	counter = 0;
        for (i = devicelist.begin(); i != devicelist.end(); i++) {
		counter++;
		std::cout << "Device: " << counter << std::endl;
		std::cout << "on " << **i << std::endl;
		DeviceDescriptorPtr     dd = (*i)->descriptor();
		std::cout << *dd << std::endl;
		ConfigurationPtr        c = (*i)->activeConfig();
		std::cout << *c;
        }
}
