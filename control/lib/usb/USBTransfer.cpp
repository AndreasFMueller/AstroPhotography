/*
 * USBTransfer.cpp -- transfer implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <AstroDebug.h>
#include <AstroUVC.h>
#include <AstroFormat.h>
#include <cstdlib>
#include <USBDebug.h>

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
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "set timeout to %d", timeout);
}

bool	Transfer::isComplete() const {
	return complete;
}

libusb_context	*Transfer::getContext() {
	libusb_context  *ctx
                = endpoint->device().getContext()->context();
	return ctx;
}

//////////////////////////////////////////////////////////////////////
// BulkTransfer implementation
//////////////////////////////////////////////////////////////////////
static void bulktransfer_callback(libusb_transfer *transfer) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "USB bulk transfer callback %p",
		transfer);
	((BulkTransfer *)transfer->user_data)->callback(transfer);
}

void	BulkTransfer::init(int _length, unsigned char *_data) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "%s transfer on %02x, size %d",
		(endpoint->bEndpointAddress() & 0x80) ? "IN" : "OUT",
		(endpoint->bEndpointAddress()), _length);
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
 * \brief convert the status to a string
 */
static const char	*usb_status_name(int status) {
	switch (status) {
	case LIBUSB_TRANSFER_ERROR:
		return "transfer error";
	case LIBUSB_TRANSFER_TIMED_OUT:
		return "transfer timed out";
	case LIBUSB_TRANSFER_CANCELLED:
		return "transfer cancelled";
	case LIBUSB_TRANSFER_STALL:
		return "transfer stall";
	case LIBUSB_TRANSFER_NO_DEVICE:
		return "transfer no device";
	case LIBUSB_TRANSFER_OVERFLOW:
		return "transfer overflow";
	case LIBUSB_TRANSFER_COMPLETED:
		return NULL;
	}
	return "UNKNOWN";
}

/**
 * \brief Submit a bulk transfer to the device.
 *
 * This method allocates and fills the bulk transfer, and submits it to
 * the libusb_submit_transfer function. It then starts to handle events,
 * and only returns when the complete flag is set.
 * \param dev_handle    the libusb device handle
 */
void	BulkTransfer::submit(libusb_device_handle *dev_handle) {
	// allocate the transfer structure
	transfer = libusb_alloc_transfer(0);

	// fill the transfer
	libusb_fill_bulk_transfer(transfer, dev_handle,
		endpoint->bEndpointAddress(), data,
		length, bulktransfer_callback, this, timeout);

	// submit the transfer
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0,
		"submitting bulk transfer %p, timeout = %d", transfer,
		timeout);
	int	rc = libusb_submit_transfer(transfer);
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "transfer submit: %d", rc);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}

	// handle events until the complete flag is set
	libusb_context  *ctx = getContext();
	while (!complete) {
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "handle events");
		libusb_handle_events(ctx);
	}

	// at this point, the transfer has somehow completed, but we
	// don't know yet what happened.
	const char	*cause = usb_status_name(transfer->status);
	if (NULL != cause) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "transfer failed: %s", cause);
		throw USBError(cause);
	} else {
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "transfer complete, %d bytes",
			transfer->actual_length);
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
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0,
		"callback: transfer status: %d, %s %d bytes",
		transfer->status,
		(endpoint->bEndpointAddress() & 0x80) ? "got" : "sent",
		transfer->actual_length);
	complete = true;
}


} // namespace usb
} // namespace astro
