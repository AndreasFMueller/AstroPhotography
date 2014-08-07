/*
 * UnicapDevice.cpp -- enumerate and open Unicap devices
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUnicap.h>
#include <cstdlib>
#include <cstring>
#include <AstroDebug.h>
#include <sstream>

using namespace astro::usb;

namespace astro {
namespace unicap {

//////////////////////////////////////////////////////////////////////
// UnicapError, exception thrown when a Unicap class encounters a problem
//////////////////////////////////////////////////////////////////////

UnicapError::UnicapError(const char *cause) : std::runtime_error(cause) {
}

UnicapError::UnicapError(unicap_status_t /* status */, const char *cause)
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
	rc = unicap_reenumerate_formats(handle, &nformats);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot reenumerate formats");
	}
	rc = unicap_reenumerate_properties(handle, &nproperties);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot reenumerate properties");
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

std::string	UnicapDevice::identifier() const {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return std::string(device.identifier);
}

std::string	UnicapDevice::model_name() const {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return std::string(device.model_name);
}

std::string	UnicapDevice::vendor_name() const {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return std::string(device.vendor_name);
}

unsigned long long	UnicapDevice::model_id() const {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return device.model_id;
}

unsigned int	UnicapDevice::vendor_id() const {
	unicap_device_t	device;
	unicap_status_t	rc = unicap_get_device(handle, &device);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get device");
	}
	return device.vendor_id;
}

int	UnicapDevice::numFormats() const {
	return nformats;
}

UnicapFormat	UnicapDevice::getFormat(int /* index */) {
	unicap_format_t	format;
	unicap_status_t	rc = unicap_get_format(handle, &format);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get format");
	}
	return UnicapFormat(&format);
}

void	UnicapDevice::setFormat(UnicapFormat& format) {
	unicap_status_t	rc = unicap_set_format(handle, &(format.format));
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot set format");
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "set format %s", format.identifier().c_str());
}

int	UnicapDevice::numProperties() const {
	return nproperties;
}

UnicapPropertyPtr	UnicapDevice::getProperty(int index) {
	unicap_property_t	property;
	unicap_status_t	rc;
	rc = unicap_enumerate_properties(handle, NULL, &property, index);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get property");
	}
	UnicapProperty	*prop;
	switch (property.type) {
	case UNICAP_PROPERTY_TYPE_VALUE_LIST:
		prop = new UnicapPropertyValuelist(&property);
		break;
	case UNICAP_PROPERTY_TYPE_MENU:
		prop = new UnicapPropertyMenu(&property);
		break;
	case UNICAP_PROPERTY_TYPE_RANGE:
		prop = new UnicapPropertyRange(&property);
		break;
	case UNICAP_PROPERTY_TYPE_FLAGS:
		prop = new UnicapPropertyFlags(&property);
		break;
	case UNICAP_PROPERTY_TYPE_DATA:
		prop = new UnicapPropertyData(&property);
		break;
	case UNICAP_PROPERTY_TYPE_UNKNOWN:
		throw UnicapError("unknown property type");
		break;
	}
	return UnicapPropertyPtr(prop);
}

std::string	UnicapDevice::toString() const {
	std::ostringstream	out;
	out << identifier();
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const UnicapDevice& device) {
	return out << device.toString();
}

/**
 * \brief Callback method.
 */
void	UnicapDevice::callback(unicap_event_t /* event */,
	unicap_data_buffer_t *buffer) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "frame received");
	frames.push_back(FramePtr(new Frame(width, height, buffer->data,
		buffer->buffer_size)));
}

static void	new_frame_callback(unicap_event_t event,
	unicap_handle_t /* handle */, unicap_data_buffer_t *buffer,
	void *user_data) {
	UnicapDevice	*unicapdevice = (UnicapDevice *)user_data;
	unicapdevice->callback(event, buffer);
}

/**
 * \brief Get Frames from a Unicap camera
 *
 * \param count		number of frames to retrieve
 * \return		a vector of Frame objects
 */
std::vector<FramePtr>	UnicapDevice::getFrames(size_t count) {
	unicap_status_t	rc;
	frames.clear();

	// find the format
	unicap_format_t	format;
	rc = unicap_get_format(handle, &format);
	if (rc != STATUS_SUCCESS) {
		throw UnicapError(rc, "cannot get the format");
	}
	width = format.size.width;
	height = format.size.height;
	int	buffer_size = format.buffer_size;
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"request %d frames (%d x %d) of size %d",
		count, width, height, buffer_size);

	// set user buffer type
	format.buffer_type = UNICAP_BUFFER_TYPE_SYSTEM;
	rc = unicap_set_format(handle, &format);
	if (rc != STATUS_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot set fornat");
		throw UnicapError(rc, "cannot set format");
	}

	// register the callback
	rc = unicap_register_callback(handle, UNICAP_EVENT_NEW_FRAME,
		(unicap_callback_t)new_frame_callback, this);
	if (rc != STATUS_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot register callback");
		throw UnicapError(rc, "cannot register callback");
	}

	// start capture
	rc = unicap_start_capture(handle);
	if (rc != STATUS_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start captures");
		throw UnicapError(rc, "cannot start capture");
	}

	// waiting for enough frames to be received
	do {
		usleep(100);
	} while (frames.size() < count);

	// stop capture
	rc = unicap_stop_capture(handle);
	if (rc != STATUS_SUCCESS) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot stop captures");
		throw UnicapError(rc, "cannot stop capture");
	}

	// return the frames
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

std::string	UnicapFormat::identifier() const {
	return std::string(format.identifier);
}

int	UnicapFormat::numSizes() const {
	return format.size_count;
}

UnicapRectangle	UnicapFormat::get(int i) {
	return UnicapRectangle(&(format.sizes[i]));
}

void	UnicapFormat::setBufferType(unicap_buffer_type_t type) {
	format.buffer_type = type;
}

std::string	UnicapFormat::toString() const {
	std::ostringstream	out;
	out << identifier() << ", size = " << format.size.width
		<< " x " << format.size.height << std::endl;
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const UnicapFormat& format) {
	return out << format.toString();
}

//////////////////////////////////////////////////////////////////////
// Unicap rectangle implementation
//////////////////////////////////////////////////////////////////////
UnicapRectangle::UnicapRectangle(unicap_rect_t *_rect) {
	memcpy(&rect, _rect, sizeof(rect));
}

int	UnicapRectangle::x() const {
	return rect.x;
}

int	UnicapRectangle::y() const {
	return rect.y;
}

int	UnicapRectangle::width() const {
	return rect.width;
}

int	UnicapRectangle::height() const {
	return rect.height;
}

//////////////////////////////////////////////////////////////////////
// UnicapProperty
//////////////////////////////////////////////////////////////////////

UnicapProperty::UnicapProperty(unicap_property_t *_property) {
	memcpy(&property, _property, sizeof(unicap_property_t));
}

UnicapProperty::~UnicapProperty() {
}

std::string	UnicapProperty::toString() const {
	std::ostringstream	out;
	out << identifier() << ": ";
	return out.str();
}

std::ostream&	operator<<(std::ostream& out, const UnicapProperty& prop) {
	out << prop.toString();
	return out;
}

std::string	UnicapProperty::identifier() const {
	return std::string(property.identifier);
}

std::string	UnicapProperty::category() const {
	return std::string(property.category);
}

std::string	UnicapProperty::unit() const {
	return std::string(property.unit);
}

//////////////////////////////////////////////////////////////////////
// UnicapPropertyDouble
//////////////////////////////////////////////////////////////////////

UnicapPropertyDouble::UnicapPropertyDouble(unicap_property_t *property)
	: UnicapProperty(property) {
}

UnicapPropertyDouble::~UnicapPropertyDouble() {
}

std::string	UnicapPropertyDouble::toString() const {
	std::ostringstream	out;
	out << UnicapProperty::toString() << value();
	return out.str();
}

double	UnicapPropertyDouble::value() const {
	return property.value;
}

//////////////////////////////////////////////////////////////////////
// UnicapPropertyRange
//////////////////////////////////////////////////////////////////////

UnicapPropertyRange::UnicapPropertyRange(unicap_property_t *property)
	: UnicapPropertyDouble(property) {
	if (property->type != UNICAP_PROPERTY_TYPE_RANGE) {
		throw UnicapError("not a range property");
	}
}

UnicapPropertyRange::~UnicapPropertyRange() {
}

std::string	UnicapPropertyRange::toString() const {
	std::ostringstream	out;
	out << UnicapPropertyDouble::toString();
	out << " [" << getMin() << ", " << getMax() << "]";
	return out.str();
}

double	UnicapPropertyRange::getMin() const {
	return property.range.min;
}

double	UnicapPropertyRange::getMax() const {
	return property.range.max;
}

//////////////////////////////////////////////////////////////////////
// UnicapPropertyValuelist
//////////////////////////////////////////////////////////////////////

UnicapPropertyValuelist::UnicapPropertyValuelist(unicap_property_t *property)
	: UnicapPropertyDouble(property) {
	if (property->type != UNICAP_PROPERTY_TYPE_VALUE_LIST) {
		throw UnicapError("not a list property");
	}
}

UnicapPropertyValuelist::~UnicapPropertyValuelist() {
}

std::string	UnicapPropertyValuelist::toString() const {
	std::ostringstream	out;
	out << UnicapPropertyDouble::toString();
	out << " (" ;
	std::vector<double>	v = values();
	std::vector<double>::const_iterator	i;
	for (i = v.begin(); i != v.end(); i++) {
		if (i != v.begin()) { out << ", "; }
		out << *i;
	}
	out << ")";
	return out.str();
}

std::vector<double>	UnicapPropertyValuelist::values() const {
	std::vector<double>	v;
	for (int i = 0; i < property.value_list.value_count; i++) {
		v.push_back(property.value_list.values[i]);
	}
	return v;
}

//////////////////////////////////////////////////////////////////////
// UnicapPropertyMenu
//////////////////////////////////////////////////////////////////////

UnicapPropertyMenu::UnicapPropertyMenu(unicap_property_t *property)
	: UnicapProperty(property) {
	if (property->type != UNICAP_PROPERTY_TYPE_MENU) {
		throw UnicapError("not a menu property");
	}
}

UnicapPropertyMenu::~UnicapPropertyMenu() {
}

std::string	UnicapPropertyMenu::toString() const {
	std::ostringstream	out;
	out << identifier() << ": ";
	out << item();
	out << " (";
	std::vector<std::string>	m = items();
	std::vector<std::string>::const_iterator	i;
	for (i = m.begin(); i != m.end(); i++) {
		if (i != m.begin()) { out << ", "; }
		out << *i;
	}
	out << ")";
	return out.str();
}

std::vector<std::string>	UnicapPropertyMenu::items() const {
	std::vector<std::string>	m;
	for (int i = 0; i < property.menu.menu_item_count; i++) {
		m.push_back(std::string(property.menu.menu_items[i]));
	}
	return m;
}

std::string	UnicapPropertyMenu::item() const {
	return std::string(property.menu_item);
}

//////////////////////////////////////////////////////////////////////
// UnicapPropertyData
//////////////////////////////////////////////////////////////////////

UnicapPropertyData::UnicapPropertyData(unicap_property_t *property)
	: UnicapProperty(property) {
	if (property->type != UNICAP_PROPERTY_TYPE_DATA) {
		throw UnicapError("not a data property");
	}
}

UnicapPropertyData::~UnicapPropertyData() {
}

std::string	UnicapPropertyData::toString() const {
	std::ostringstream	out;
	out << UnicapProperty::toString();
	return out.str();
}

//////////////////////////////////////////////////////////////////////
// UnicapPropertyData
//////////////////////////////////////////////////////////////////////

UnicapPropertyFlags::UnicapPropertyFlags(unicap_property_t *property)
	: UnicapProperty(property) {
	if (property->type != UNICAP_PROPERTY_TYPE_FLAGS) {
		throw UnicapError("not a flags property");
	}
}

UnicapPropertyFlags::~UnicapPropertyFlags() {
}

std::string	UnicapPropertyFlags::toString() const {
	std::ostringstream	out;
	out << UnicapProperty::toString();
	return out.str();
}

} // namespace unicap
} // namespace astro
