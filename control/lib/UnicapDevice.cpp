/*
 * UnicapDevice.cpp -- enumerate and open Unicap devices
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUnicap.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>

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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "device opened: %s",
		identifier().c_str());
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

void	UnicapDevice::setFormat(int index) {
	unicap_format_t	format;
	unicap_status_t	rc = unicap_get_format(handle, &format);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get format");
	}
	rc = unicap_set_format(handle, &format);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot set format");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set format %s", format.identifier);
}

/**
 * \brief Get Frames from a Unicap camera
 *
 * \param count		number of frames to retrieve
 * \return		a vector of Frame objects
 */
std::vector<FramePtr>	UnicapDevice::getFrames(int count) {
	unicap_status_t	rc;

	// find the format
	unicap_format_t	format;
	rc = unicap_get_format(handle, &format);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get the format");
	}
	int	width = format.size.width;
	int	height = format.size.height;
	int	bpp = format.bpp;
	int	buffer_size = format.buffer_size;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"request %d frames (%d x %d) of size %d",
		count, width, height, buffer_size);

	// find the buffer size that we should be using
	std::vector<FramePtr>	frames;
	int	queued = 0;
	int	queuesize = 3;

	// create a few buffers and queue them
	debug(LOG_DEBUG, DEBUG_LOG, 0, "creating buffers");
	unicap_data_buffer_t	*buffers[queuesize];
	for (int i = 0; i < queuesize; i++) {
		unicap_data_buffer_t	*buffer;
		buffer = (unicap_data_buffer_t *)calloc(1,
			sizeof(unicap_data_buffer_t));
		buffers[i] = buffer;
		buffer->buffer_size = buffer_size;
		buffer->data = (unsigned char *)malloc(buffer_size);
	}

	// start capturing
	rc = unicap_start_capture(handle);
	if (rc != STATUS_SUCCESS) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "cannot start captures");
		throw UnicapError(rc, "cannot start capture");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "capture started");

	// queue the buffers
	for (int i = 0; i < queuesize; i++) {
		rc = unicap_queue_buffer(handle, buffers[i]);
		if (rc != STATUS_SUCCESS) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot queue %d", i);
			goto cleanup;
		}
		queued++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d buffers queued", queued);

	// wait for frames to arrive
	while (frames.size() < count) {
		unicap_data_buffer_t	*returned;
		rc = unicap_wait_buffer(handle, &returned);
		if (rc != STATUS_SUCCESS) {
			debug(LOG_ERR, DEBUG_LOG, 0, "cannot receive a buffer");
		}
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got buffer %d", frames.size());
		Frame	*f = new Frame(width, height, returned->data,
			returned->buffer_size);
		frames.push_back(FramePtr(f));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "new frame added");
		if (queued < count) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "requeue the buffer");
			rc = unicap_queue_buffer(handle, returned);
			queued++;
		}
	}
	rc = unicap_stop_capture(handle);

cleanup:
	for (int i = 0; i < queuesize; i++) {
		free(buffers[i]->data);
		buffers[i]->data = NULL;
		free(buffers[i]);
		buffers[i] = NULL;
	}
	free(buffers);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "capture stopped");
	return frames;
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

//////////////////////////////////////////////////////////////////////
// Unicap Properties
//////////////////////////////////////////////////////////////////////

} // namespace unicap
} // namespace astro
