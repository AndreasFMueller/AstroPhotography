/*
 * hidapi.c -- this file just pulls in the correct driver for the plattform
 *
 * (c) 2018 Prof Dr Andreas Müller, HIDAPI Driver
 */
#include <hidapi.h>

#ifdef __APPLE__
#include "hid-macos.c"
#elif __linux__
#include "hid-libusb.c"
#else
#error "Compiler not supported for hidapi
#endif
