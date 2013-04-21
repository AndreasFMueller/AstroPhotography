/*
 * UnicapDevice.cpp -- enumerate and open Unicap devices
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUnicap.h>
#include <stdlib.h>
#include <string.h>

namespace astro {
namespace unicap {

//////////////////////////////////////////////////////////////////////
// UnicapError, exception thrown when a Unicap class encounters a problem
//////////////////////////////////////////////////////////////////////

UnicapError::UnicapError(const char *cause) : std::runtime_error(cause) {
}

UnicapError::UnicapError(unicap_status_t status, const char *cause)
	: std::runtime_error(cause) {
}

//////////////////////////////////////////////////////////////////////
// UnicapError, exception thrown when a Unicap class encounters a problem
//////////////////////////////////////////////////////////////////////

Unicap::Unicap() {
}

int	Unicap::numDevices() {
	int	count;
	unicap_status_t	rc = unicap_reenumerate_devices(&count);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot enumerate devices");
	}
	return count;
}

UnicapDevice	Unicap::get(int index) {
	if ((index < 0) || (index >= numDevices())) {
		throw std::range_error("out of device range");
	}
	unicap_device_t	device;
	unicap_status_t	rc = unicap_enumerate_devices(NULL, &device, index);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot enumerate device");
	}
	return UnicapDevice(&device);
}

//////////////////////////////////////////////////////////////////////
// UnicapDevice class
//////////////////////////////////////////////////////////////////////

UnicapDevice::UnicapDevice(unicap_device_t *device) {
	unicap_status_t	rc = unicap_open(&handle, device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot open the device");
	}
}

UnicapDevice::UnicapDevice(const UnicapDevice& other) {
	handle = unicap_clone_handle(other.handle);
}

UnicapDevice::~UnicapDevice() {
	if (isopen) {
		unicap_close(handle);
		isopen = false;
	}
}

std::string	UnicapDevice::identifier() {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return std::string(device.identifier);
}

std::string	UnicapDevice::model_name() {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return std::string(device.model_name);
}

std::string	UnicapDevice::vendor_name() {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return std::string(device.vendor_name);
}

unsigned long long	UnicapDevice::model_id() {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return device.model_id;
}

unsigned int	UnicapDevice::vendor_id() {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return device.vendor_id;
}

int	UnicapDevice::numFormats() {
	int	count;
	unicap_status_t	rc = unicap_reenumerate_formats(handle, &count);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot reenumerate formats");
	}
	return count;
}

UnicapFormat	UnicapDevice::getFormat(int index) {
	unicap_format_t	format;
	unicap_status_t	rc = unicap_get_format(handle, &format);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get format");
	}
	return UnicapFormat(&format);
}

//////////////////////////////////////////////////////////////////////
// Unicap format implementation
//////////////////////////////////////////////////////////////////////

UnicapFormat::UnicapFormat(unicap_format_t *_format) {
	memcpy(&format, _format, sizeof(format));
	format.sizes = (unicap_rect_t *)calloc(_format->size_count,
		sizeof(unicap_rect_t));
	memcpy(format.sizes, _format->sizes,
		_format->size_count * sizeof(unicap_rect_t));
}

UnicapFormat::UnicapFormat(const UnicapFormat& other) {
	memcpy(&format, &other.format, sizeof(unicap_format_t));
	format.sizes = (unicap_rect_t *)calloc(other.format.size_count,
		sizeof(unicap_rect_t));
	memcpy(format.sizes, other.format.sizes,
		format.size_count * sizeof(unicap_rect_t));
}

UnicapFormat::~UnicapFormat() {
	free(format.sizes);
}

std::string	UnicapFormat::identifier() {
	return std::string(format.identifier);
}

int	UnicapFormat::numSizes() {
	return format.size_count;
}

UnicapRectangle	UnicapFormat::get(int i) {
	return UnicapRectangle(&(format.sizes[i]));
}

//////////////////////////////////////////////////////////////////////
// Unicap rectangle implementation
//////////////////////////////////////////////////////////////////////
UnicapRectangle::UnicapRectangle(unicap_rect_t *_rect) {
	memcpy(&rect, _rect, sizeof(rect));
}

int	UnicapRectangle::x() {
	return rect.x;
}

int	UnicapRectangle::y() {
	return rect.y;
}

int	UnicapRectangle::width() {
	return rect.width;
}

int	UnicapRectangle::height() {
	return rect.height;
}

} // namespace unicap
} // namespace astro
