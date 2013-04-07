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
Transfer::Transfer(uint8_t _endpoint, int _length, unsigned char *_data)
	: endpoint(_endpoint), length(_length) {
	transfer = NULL;
	if (_data == NULL) {
		data = new unsigned char[length];
		freedata = true;
	} else {
		data = _data;
		freedata = false;
	}
	timeout = 1000;
}

Transfer::~Transfer() {
	if (transfer) {
		libusb_free_transfer(transfer);
		transfer = NULL;
	}
	if ((NULL != data) && (freedata)) {
		delete[] data;
	}
	data = NULL;
}

int	Transfer::getTimeout() const {
	return timeout;
}

void	Transfer::setTimeout(int _timeout) {
	timeout = _timeout;
}

//////////////////////////////////////////////////////////////////////
// BulkTransfer implementation
//////////////////////////////////////////////////////////////////////
static void bulktransfer_callback(libusb_transfer *transfer) {
	((BulkTransfer *)transfer->user_data)->callback();
}

BulkTransfer::BulkTransfer(uint8_t _endpoint, int _length, unsigned char *_data)
	: Transfer(_endpoint, _length, _data) {
}

void	BulkTransfer::submit(libusb_device_handle *dev_handle) throw(USBError) {
	// allocate the transfer structure
	transfer = libusb_alloc_transfer(0);

	// fill the transfer
	libusb_fill_bulk_transfer(transfer, dev_handle, endpoint, data,
		length, bulktransfer_callback, this, timeout);

	// submit the transfer
	int	rc = libusb_submit_transfer(transfer);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

BulkTransfer::~BulkTransfer() {
}

void	BulkTransfer::callback() {
	std::cout << "bulk transfer callback" << std::endl;
}

//////////////////////////////////////////////////////////////////////
// IsoTransfer implementation
//////////////////////////////////////////////////////////////////////
static void isotransfer_callback(libusb_transfer *transfer) {
	((IsoTransfer *)transfer->user_data)->callback();
}

IsoTransfer::IsoTransfer(uint8_t _endpoint, int _length, unsigned char *_data)
	: Transfer(_endpoint, _length, _data) {
}

void	IsoTransfer::submit(libusb_device_handle *dev_handle) throw(USBError) {
	// allocate the transfer structure
	int	packets = 0;
	transfer = libusb_alloc_transfer(packets);

	// fill the transfer
	libusb_fill_iso_transfer(transfer, dev_handle, endpoint, data,
		length, packets, isotransfer_callback, this, timeout);

	// submit the transfer
	int	rc = libusb_submit_transfer(transfer);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

IsoTransfer::~IsoTransfer() {
}

void	IsoTransfer::callback() {
	std::cout << "iso transfer callback" << std::endl;
}


} // namespace usb
} // namespace astro
