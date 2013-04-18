/*
 * USBTransfer.cpp -- transfer implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <debug.h>
#include <AstroUVC.h>
#include <Format.h>
#include <stdlib.h>

namespace astro {
namespace usb {

//////////////////////////////////////////////////////////////////////
// Transfer implementation
//////////////////////////////////////////////////////////////////////
Transfer::Transfer(EndpointDescriptorPtr _endpoint,
	int _length, unsigned char *_data)
	: endpoint(_endpoint), length(_length) {
	if (_data == NULL) {
		data = new unsigned char[length];
		freedata = true;
	} else {
		data = _data;
		freedata = false;
	}
	timeout = 1000;
	complete = false;
}

Transfer::~Transfer() {
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

bool	Transfer::isComplete() const {
	return complete;
}

//////////////////////////////////////////////////////////////////////
// BulkTransfer implementation
//////////////////////////////////////////////////////////////////////
static void bulktransfer_callback(libusb_transfer *transfer) {
	((BulkTransfer *)transfer->user_data)->callback();
}

BulkTransfer::BulkTransfer(EndpointDescriptorPtr _endpoint,
	int _length, unsigned char *_data)
	: Transfer(_endpoint, _length, _data) {
	transfer = NULL;
}

void	BulkTransfer::submit(libusb_device_handle *dev_handle) throw(USBError) {
	// allocate the transfer structure
	transfer = libusb_alloc_transfer(0);

	// fill the transfer
	libusb_fill_bulk_transfer(transfer, dev_handle,
		endpoint->bEndpointAddress(), data,
		length, bulktransfer_callback, this, timeout);

	// submit the transfer
	int	rc = libusb_submit_transfer(transfer);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}
}

BulkTransfer::~BulkTransfer() {
	if (transfer) {
		libusb_free_transfer(transfer);
		transfer = NULL;
	}
}

void	BulkTransfer::callback() {
	std::cout << "bulk transfer callback" << std::endl;
}


} // namespace usb
} // namespace astro
