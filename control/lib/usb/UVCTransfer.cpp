/*
 * UVCTransfer.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUVC.h>
#include <sstream>
#include <iomanip>
#include <AstroDebug.h>
#include <cstdlib>

namespace astro {
namespace usb {
namespace uvc {

//////////////////////////////////////////////////////////////////////
// UVCBulkTransfer implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Callback for UVC bulk transfers.
 *
 * This function simple redirects the callback to the callback method
 * to the UVCBulkTransfer instance.
 */
static void uvcbulk_callback(struct libusb_transfer *transfer) {
	UVCBulkTransfer	*uvcbulktransfer
		= (UVCBulkTransfer *)transfer->user_data;
	uvcbulktransfer->callback(transfer);
}

/**
 * \brief Create a UVCBulkTransfer.
 *
 * 
 * \param endpoint
 * \param _payloadtransfersize
 * \param _maxframesize
 */
UVCBulkTransfer::UVCBulkTransfer(EndpointDescriptorPtr endpoint, int _nframes,
	size_t _payloadtransfersize, size_t _maxframesize)
	: Transfer(endpoint),
	  payloadtransfersize(_payloadtransfersize),
	  maxframesize(_maxframesize), nframes(_nframes) {
	submitted = 0;

	// compute the number of transfers required for all the frames
	// we are asking for
	int	ptspfrm = (1 + maxframesize / (payloadtransfersize - 12));
	ntransfers = ptspfrm * (nframes + 1);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need %d transfers to get %d frames",
		ntransfers, nframes);
	queuesize = 2; // depends on architecture

	// create transfers for all those packets
	transfers = (struct libusb_transfer **)calloc(ntransfers,
		sizeof(struct libusb_transfer *));
	buffers = (unsigned char **)calloc(ntransfers, sizeof(unsigned char *));
	for (int i = 0; i < queuesize; i++) {
		transfers[i] = libusb_alloc_transfer(0);
		buffers[i] = (unsigned char *)malloc(payloadtransfersize + 12);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d transfers/buffers allocated",
		ntransfers);
}

/**
 * \brief Destructor cleans up all allocations
 */
UVCBulkTransfer::~UVCBulkTransfer() {
	for (int i = 0; i < ntransfers; i++) {
		libusb_free_transfer(transfers[i]);
		free(buffers[i]);
	}
	free(buffers);
	free(transfers);
}

/**
 * \brief 
 *
 * \param devhandle
 */
void	UVCBulkTransfer::submit(libusb_device_handle *devhandle) {
	// fill all the transfers
	for (int i = 0; i < queuesize; i++) {
		libusb_fill_bulk_transfer(transfers[i], devhandle,
			endpoint->bEndpointAddress(), buffers[i],
			payloadtransfersize + 12, uvcbulk_callback, this,
			timeout);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfers filled: %d", ntransfers);

	// now submit them all
	for (int i = 0; i < queuesize; i++) {
		libusb_submit_transfer(transfers[i]);
		submitted++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfers submitted: %d", submitted);

	// now handle events until all transfers are done
	libusb_context	*context = getContext();
	int	outstanding = ntransfers;
	while (outstanding) {
		libusb_handle_events(context);
		outstanding--;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "transfers outstanding: %d",
			outstanding);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfer complete");
}

/**
 * \brief Callback for UVC bulk transfers
 *
 * The callback just packs the packets into the packet list.
 */
void	UVCBulkTransfer::callback(libusb_transfer *transfer) {
	debug(LOG_DEBUG, DEBUG_LOG, 0,
		"UVCBulkTransfer callback: %d bytes", transfer->actual_length);
	if (transfer->actual_length >= 12) {
		// create a new payload packet from the transferred data
		packets.push_back(std::string((char *)transfer->buffer,
			transfer->actual_length));
	} else {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "ignoring short packet: %d",
			transfer->actual_length);
	}
	if (submitted < ntransfers) {
		libusb_submit_transfer(transfer);
		submitted++;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "return from callback");
}
 
//////////////////////////////////////////////////////////////////////
// UVCIsochronousTransfer implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Callback for UVC isochronous transfers.
 *
 * This function simple redirects the callback to the callback method
 * to the UVCIsochronousTransfer instance.
 */
static void uvcisochronous_callback(struct libusb_transfer *transfer) {
	UVCIsochronousTransfer	*uvcisochronoustransfer
		= (UVCIsochronousTransfer *)transfer->user_data;
	uvcisochronoustransfer->callback(transfer);
}

/**
 * \brief Callback for UVC isochronous transfers
 *
 * The callback just packs the packets into the packet list.
 * \param transfer	the currently processed transfer
 */
void	UVCIsochronousTransfer::callback(libusb_transfer *transfer) {
	unsigned long	bytes = 0;
	// go through all the iso packets
	for (int i = 0; i < transfer->num_iso_packets; i++) {
		int	length = transfer->iso_packet_desc[i].actual_length;
		int	status = transfer->iso_packet_desc[i].status;
		if ((0 == status) && (length >= 12)) {
			char	*data = (char *)libusb_get_iso_packet_buffer_simple(transfer, i);
			// create a new payload packet from the transferred data
			packets.push_back(std::string(data, length));

			// count the data bytes (not frame headers) transferred
			bytes += length - 12;
			bytestransferred += length - 12;
		}
	}

	// if some bytes where transferred, then this is a completed
	// transfer
	if (bytes > 0) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "got %lu bytes", bytes);
		completed++;
	}

	// resubmit the transfer if necessary
	if (completed < ntransfers) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "resubmitting %p", transfer);
		libusb_submit_transfer(transfer);
		submitted++;
	}
}
 
/**
 * \brief Create a UVCIsochronousTransfer.
 *
 * 
 * \param endpoint
 * \param _payloadtransfersize
 * \param _maxframesize
 */
static int	isochunk = 400;
UVCIsochronousTransfer::UVCIsochronousTransfer(EndpointDescriptorPtr endpoint,
	int _nframes, int _frameinterval)
	: Transfer(endpoint), nframes(_nframes),
	  frameinterval(_frameinterval) {
	submitted = 0;
	bytestransferred = 0;
	completed = 0;

	// the frame interval and the number of frames tells us how many
	// transfer microframes we need
	double	microframesperframe = frameinterval / 1250.;
	int	isoframes = microframesperframe * (1 + nframes);
	isoframes = isochunk * (1 + (isoframes / isochunk));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "isoframes = %d", isoframes);

	// compute the packet size
	packetsize = endpoint->maxPacketSize()
			* endpoint->transactionOpportunities();
	int	buffersize = packetsize * isochunk;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "iso packetsize = %d, buffersize = %d",
		packetsize, buffersize);

	// find out how many isochronous transfers we need
	ntransfers = 2 + isoframes / isochunk;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "need %d transfers to get %d frames",
		ntransfers, nframes);
	queuesize = 4; // depends on architecture

	// create transfers for all those packets
	transfers = (struct libusb_transfer **)calloc(ntransfers,
		sizeof(struct libusb_transfer *));
	buffers = (unsigned char **)calloc(ntransfers, sizeof(unsigned char *));
	for (int i = 0; i < queuesize; i++) {
		transfers[i] = libusb_alloc_transfer(isochunk);
		transfers[i]->type = LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;
		transfers[i]->num_iso_packets = isochunk;
		buffers[i] = (unsigned char *)malloc(buffersize);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "%d transfers/buffers allocated",
		ntransfers);
}

/**
 * \brief Destructor cleans up all allocations
 */
UVCIsochronousTransfer::~UVCIsochronousTransfer() {
	for (int i = 0; i < ntransfers; i++) {
		libusb_free_transfer(transfers[i]);
		free(buffers[i]);
	}
	free(buffers);
	free(transfers);
}

/**
 * \brief Submit an isochronous transfer.
 *
 * \param devhandle	libusb device handle to use during transfer.
 */
void	UVCIsochronousTransfer::submit(libusb_device_handle *dev_handle) {
	// fill all the transfers
	int	buffersize = packetsize * isochunk;
	for (int i = 0; i < queuesize; i++) {
		libusb_fill_iso_transfer(transfers[i], dev_handle,
			endpoint->bEndpointAddress(), buffers[i], buffersize,
			isochunk, uvcisochronous_callback, this, timeout);
		libusb_set_iso_packet_lengths(transfers[i], packetsize);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfers filled: %d", queuesize);

	// now submit them all
	for (int i = 0; i < queuesize; i++) {
		libusb_submit_transfer(transfers[i]);
		submitted++;
		debug(LOG_DEBUG, DEBUG_LOG, 0, "submitted: %p", transfers[i]);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfers submitted: %d", submitted);

	// now handle events until all transfers are done
	libusb_context	*context = getContext();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "completed = %d, ntransfers = %d",
		completed, ntransfers);
	while (completed < ntransfers) {
		libusb_handle_events(context);
		debug(LOG_DEBUG, DEBUG_LOG, 0, "completed: %d", completed);
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "transfer complete");
}

 
//////////////////////////////////////////////////////////////////////
// UVCPayloadPacket implementation
//////////////////////////////////////////////////////////////////////
/**
 * \brief Create a payload packet from a data block.
 */
UVCPayloadPacket::UVCPayloadPacket(const std::string& _data) : data(_data) {
	if (data.size() < 12) {
		throw std::length_error("must have at least 12 bytes");
	}
}

uint8_t	UVCPayloadPacket::hle() const {
	return (uint8_t)data.data()[0];
}

uint8_t	UVCPayloadPacket::bfh() const {
	return (uint8_t)data.data()[1];
}

bool	UVCPayloadPacket::eoh() const {
	return ((1 << 7) & (uint8_t)data.data()[1]) ? true : false;
}

bool	UVCPayloadPacket::err() const {
	return ((1 << 6) & (uint8_t)data.data()[1]) ? true : false;
}

bool	UVCPayloadPacket::sti() const {
	return ((1 << 5) & (uint8_t)data.data()[1]) ? true : false;
}

bool	UVCPayloadPacket::res() const {
	return ((1 << 4) & (uint8_t)data.data()[1]) ? true : false;
}

bool	UVCPayloadPacket::scr() const {
	return ((1 << 3) & (uint8_t)data.data()[1]) ? true : false;
}

bool	UVCPayloadPacket::pts() const {
	return ((1 << 2) & (uint8_t)data.data()[1]) ? true : false;
}

bool	UVCPayloadPacket::eof() const {
	return ((1 << 1) & (uint8_t)data.data()[1]) ? true : false;
}

bool	UVCPayloadPacket::fid() const {
	return ((1 << 0) & (uint8_t)data.data()[1]) ? true : false;
}

uint32_t	UVCPayloadPacket::ptsValue() const {
	return *(uint32_t *)&data.data()[2];
}

uint64_t	UVCPayloadPacket::scrValue() const {
	uint64_t	result = 0;
	if (scr()) {
		memcpy(&result, &data.data()[7], 6);
	}
	return result;
}

std::string	UVCPayloadPacket::payload() const {
	return data.substr((int)hle());
}


} // namespace uvc
} // namespace usb
} // namespace astro
