/*
 * USBTransfer.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>

namespace astro {
namespace usb {

//////////////////////////////////////////////////////////////////////
// Transfer implementation
//////////////////////////////////////////////////////////////////////
Transfer::Transfer(DeviceHandle& handle, int _length)
	: devicehandle(handle), length(_length) {
	transfer = NULL;
	data = new unsigned char[length];
}

Transfer::~Transfer() {
	if (transfer) {
		libusb_free_transfer(transfer);
		transfer = NULL;
	}
	if (NULL != data) {
		delete[] data;
	}
}

libusb_device_handle	*Transfer::handle() {
	return devicehandle.dev_handle;
}

void	Transfer::submit() throw(USBError) {
	int	rc = libusb_submit_transfer(transfer);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

//////////////////////////////////////////////////////////////////////
// BulkTransfer implementation
//////////////////////////////////////////////////////////////////////
static void bulktransfer_callback(libusb_transfer *transfer) {
	((BulkTransfer *)transfer->user_data)->callback();
}

BulkTransfer::BulkTransfer(DeviceHandle &_handle, uint8_t endpoint,
	int _length)
	: Transfer(_handle, _length) {
	transfer = libusb_alloc_transfer(0);
	libusb_fill_bulk_transfer(transfer, handle(),
		endpoint, data, length,
		bulktransfer_callback, this, 1000);
}

BulkTransfer::~BulkTransfer() {
}

void	BulkTransfer::callback() {
}

//////////////////////////////////////////////////////////////////////
// IsoTransfer implementation
//////////////////////////////////////////////////////////////////////
IsoTransfer::IsoTransfer(DeviceHandle &_handle, int _length)
	: Transfer(_handle, _length) {
	// find out how many packets we will need
	transfer = libusb_alloc_transfer(0);
}

IsoTransfer::~IsoTransfer() {
}

void	IsoTransfer::callback() {
}


} // namespace usb
} // namespace astro
