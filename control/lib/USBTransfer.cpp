/*
 * USBTransfer.cpp -- transfer implementation
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <debug.h>
#include <AstroUVC.h>

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
	complete = false;
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found packet size: %d", packetsize);

	// compute the number of packets we are expecting
	packets = (2 * length) / packetsize;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "expecting about %d packets", packets);

	// allocate the transfer structure
	transfer = libusb_alloc_transfer(packets);
	if (NULL == transfer) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot allocate transfer");
		throw std::runtime_error("cannot allocate transfer");
	}
	transfer->type = LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;
	transfer->num_iso_packets = packets;

	// fill the transfer request structure
	libusb_fill_iso_transfer(transfer, dev_handle,
		endpoint->bEndpointAddress(), data,
		length, packets, isotransfer_callback, this, timeout);

	// initialize the iso packet lengths
	libusb_set_iso_packet_lengths(transfer, packetsize);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "packet length set to %d", packetsize);
	packetcounter = 0;

#if 0
	// claim the interface (again)
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reclaim interface");
	endpoint->interface().getInterface().claim();
#endif
	// submit the transfer
	debug(LOG_DEBUG, DEBUG_LOG, 0, "libusb_submit_transfer(%p)", transfer);
	int	rc = libusb_submit_transfer(transfer);
	if (rc != LIBUSB_SUCCESS) {
		throw USBError(libusb_error_name(rc));
	}

	// display the packet headers
#if 0
	for (int packetno = 0; packetno < packets; packetno++) {
		debug(LOG_DEBUG, DEBUG_LOG, 0,
			"packet[%d]: length = %d, actual_length = %d, "
				"status = %d",
			packetno,
			transfer->iso_packet_desc[packetno].length,
			transfer->iso_packet_desc[packetno].actual_length,
			transfer->iso_packet_desc[packetno].status);
	}
#endif

	// waiting for the transfer to complete
	libusb_context	*ctx
		= endpoint->device().getContext()->getLibusbContext();
	while (!this->isComplete()) {
		int	rc = libusb_handle_events(ctx);
		if (rc < 0) {
			debug(LOG_ERR, DEBUG_LOG, 0, "event handling error: %s",
				libusb_error_name(rc));
			throw USBError(libusb_error_name(rc));
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait for event handling");
	sleep(10);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "wait complete");
}

IsoTransfer::~IsoTransfer() {
}

void	IsoTransfer::callback() {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "iso callback");
	debug(LOG_DEBUG, DEBUG_LOG, 0, "status = %d",
		transfer->flags);
	int	headercounter = 0;
	int	datacounter = 0;
	for (int packetno = packetcounter; packetno < packets; packetno++) {
		if (transfer->iso_packet_desc[packetno].actual_length) {
#if 0
			debug(LOG_DEBUG, DEBUG_LOG, 0,
				"packet[%d]: length = %d, actual_length = %d, "
				"status = %d",
				packetno,
				transfer->iso_packet_desc[packetno].length,
				transfer->iso_packet_desc[packetno].actual_length,
				transfer->iso_packet_desc[packetno].status);
#endif
		}
		if (12 == transfer->iso_packet_desc[packetno].actual_length) {
			astro::usb::uvc::stream_header_t	*h = (astro::usb::uvc::stream_header_t *)libusb_get_iso_packet_buffer(transfer, packetno);
			headercounter++;
			//std::cout << astro::usb::uvc::stream_header2string(*h);
		}
		if (transfer->iso_packet_desc[packetno].actual_length > 12) {
			debug(LOG_DEBUG, DEBUG_LOG, 0, "data packet: status %d",
				transfer->iso_packet_desc[packetno].status);
			datacounter++;
		}
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "headers: %d, data: %d",
		headercounter, datacounter);
	packetcounter++;
}


} // namespace usb
} // namespace astro
