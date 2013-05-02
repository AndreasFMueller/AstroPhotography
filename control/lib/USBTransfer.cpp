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
Transfer::Transfer(EndpointDescriptorPtr _endpoint)
	: endpoint(_endpoint) {
	timeout = 1000;
	complete = false;
}

Transfer::~Transfer() {
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

libusb_context	*Transfer::getContext() {
	libusb_context  *ctx
                = endpoint->device().getContext()->getLibusbContext();
	return ctx;
}

//////////////////////////////////////////////////////////////////////
// BulkTransfer implementation
//////////////////////////////////////////////////////////////////////
static void bulktransfer_callback(libusb_transfer *transfer) {
	((BulkTransfer *)transfer->user_data)->callback(transfer);
}

void	BulkTransfer::init(int _length, unsigned char *_data) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%s transfer on %02x",
		(endpoint->bEndpointAddress() & 0x80) ? "IN" : "OUT",
		(endpoint->bEndpointAddress()));
	transfer = NULL;
	length = _length;
	if (NULL != _data) {
		data = _data;
		freedata = false;
	} else {
		data = new unsigned char[_length];
		freedata = true;
	}
}

BulkTransfer::BulkTransfer(EndpointDescriptorPtr _endpoint,
	int _length, unsigned char *_data)
	: Transfer(_endpoint) {
	init(_length, _data);
}

unsigned char	*BulkTransfer::getData() const {
	return data;
}

/**
 * \brief Submit a bulk transfer to the device.
 *
 * This method allocates and fills the bulk transfer, and submits it to
 * the libusb_submit_transfer function. It then starts to handle events,
 * and only returns when the complete flag is set.
 * \param dev_handle    the libusb device handle
 */
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

	// handle events until the complete flag is set
	libusb_context  *ctx = getContext();
	while (!complete) {
		libusb_handle_events(ctx);
	}
}

/**
 * \brief Destroy the Bulk transfer
 *
 * The object may not go out of scope when a transfer is still active.
 */
BulkTransfer::~BulkTransfer() {
	if (transfer) {
		libusb_free_transfer(transfer);
		transfer = NULL;
	}
	if (freedata) {
		delete [] data;
		data = NULL;
	}
}

/**
 * \brief Default callback.
 *
 * This callback just accepts the transfer so far that everything has
 * been transferred and sets the complete flag. If this is not the
 * intended behaviour, this class should be subclassed and this
 * method overridden.
 */
void	BulkTransfer::callback(libusb_transfer *transfer) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfer complete: %s %d bytes",
		(endpoint->bEndpointAddress() & 0x80) ? "got" : "sent",
		transfer->actual_length);
	complete = true;
}


} // namespace usb
} // namespace astro
