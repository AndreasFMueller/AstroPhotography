/*
 * USBTransfer.cpp -- transfer implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <debug.h>

namespace astro {
namespace usb {

//////////////////////////////////////////////////////////////////////
// Transfer implementation
//////////////////////////////////////////////////////////////////////
Transfer::Transfer(EndpointDescriptorPtr _endpoint,
	int _length, unsigned char *_data)
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

BulkTransfer::BulkTransfer(EndpointDescriptorPtr _endpoint,
	int _length, unsigned char *_data)
	: Transfer(_endpoint, _length, _data) {
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
}

void	BulkTransfer::callback() {
	std::cout << "bulk transfer callback" << std::endl;
}

//////////////////////////////////////////////////////////////////////
// IsoTransfer implementation
//////////////////////////////////////////////////////////////////////
static void isotransfer_callback(libusb_transfer *transfer) {
	((IsoTransfer *)transfer->user_data)->callback();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "iso transfer callback");
}

IsoTransfer::IsoTransfer(EndpointDescriptorPtr _endpoint,
	int _length, unsigned char *_data)
	: Transfer(_endpoint, _length, _data) {
}

/**
 * \brief Isochronous transfer implementation
 *
 * \param dev_handle	libusb_device_handle to use for the transfer
 */
void	IsoTransfer::submit(libusb_device_handle *dev_handle) throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing isochronous transfer");

	// find the packet size that the endpoint can handle
	int	packetsize = endpoint->maxPacketSize()
			* endpoint->transactionOpportunities();
	packetsize = 5120;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found packet size: %d", packetsize);

	// compute the number of packets we are expecting
	int	packets = (2 * length) / packetsize;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "expecting about %d packets", packets);

	// allocate the transfer structure
	transfer = libusb_alloc_transfer(packets);
	if (NULL == transfer) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot allocate transfer");
		throw std::runtime_error("cannot allocate transfer");
	}

	// fill the transfer request structure
	libusb_fill_iso_transfer(transfer, dev_handle,
		endpoint->bEndpointAddress(), data,
		length, packets, isotransfer_callback, this, timeout);

	// initialize the iso packet lengths
#if 1
	libusb_set_iso_packet_lengths(transfer, packetsize);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "packet length set to %d", packetsize);
#endif

	// claim the interface (again)
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reclaim interface");
	endpoint->interface().getInterface().claim();

	// submit the transfer
	debug(LOG_DEBUG, DEBUG_LOG, 0, "libusb_submit_transfer(%p)", transfer);
	int	rc = libusb_submit_transfer(transfer);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}

	// waiting for the transfer to complete
	//libusb_handle_events(device.getContext().getLibusbContext());
}

IsoTransfer::~IsoTransfer() {
}

void	IsoTransfer::callback() {
	std::cout << "iso transfer callback" << std::endl;
}


} // namespace usb
} // namespace astro
