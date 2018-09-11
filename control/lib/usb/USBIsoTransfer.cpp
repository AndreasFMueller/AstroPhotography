/*
 * USBIsoTransfer.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <AstroDebug.h>
#include <cstdlib>
#include <USBDebug.h>

namespace astro {
namespace usb {

#if 0
//////////////////////////////////////////////////////////////////////
// IsoPacket implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct an IsoPacket from the data
 */
IsoPacket::IsoPacket(unsigned char *data, size_t length, int _status)
	: std::string((char *)data, length), status(_status) {
}
#endif

//////////////////////////////////////////////////////////////////////
// IsoSegment implementation
//////////////////////////////////////////////////////////////////////

static void isotransfer_callback(libusb_transfer *transfer) {
	IsoTransfer	*isotransferptr = (IsoTransfer *)transfer->user_data;
	isotransferptr->callback(transfer);
}

/**
 * \brief Create an isochronous segment.
 *
 * \param _endpoint
 * \param packets
 * \param _isotransfer
 * \param dev_handle
 * \param timeout
 */
IsoSegment::IsoSegment(EndpointDescriptorPtr _endpoint, int packets,
	IsoTransfer *_isotransfer, libusb_device_handle *dev_handle,
	int timeout) : endpoint(_endpoint), isotransfer(_isotransfer) {
	// compute the packet size for the packets that we need
	int	packetsize = endpoint->maxPacketSize()
			* endpoint->transactionOpportunities();

	// allocate the transfer
	transfer = libusb_alloc_transfer(packets);
	if (NULL == transfer) {
		USBdebug(LOG_ERR, DEBUG_LOG, 0, "cannot allocate transfer");
	}
	transfer->type = LIBUSB_TRANSFER_TYPE_ISOCHRONOUS;
	transfer->num_iso_packets = packets;

	// create a buffer for the data
	size_t	buffersize = packets * packetsize;
	unsigned char	*buffer = (unsigned char *)calloc(1, buffersize);
	transfer->flags |= LIBUSB_TRANSFER_FREE_BUFFER;

	// fill the transfer request structure
	libusb_fill_iso_transfer(transfer, dev_handle,
		endpoint->bEndpointAddress(), buffer, buffersize,
		packets, isotransfer_callback, isotransfer, timeout);

	// set the packet sizes, all have the same size
	libusb_set_iso_packet_lengths(transfer, packetsize);

	// log completion of the segment creation
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "created IsoSegment with %d packets",
		packets);
}

/**
 * \brief Destroy an iso packet 
 */
IsoSegment::~IsoSegment() {
	libusb_free_transfer(transfer);
	transfer = NULL;
}

/**
 * \brief Submit an IsoSegment 
 */
void	IsoSegment::submit() {
	libusb_submit_transfer(transfer);
}

int	IsoSegment::extract(std::list<std::string>& packets) {
	int	packetcounter = 0;
	for (int i = 0; i < transfer->num_iso_packets; i++) {
		if (0 == transfer->iso_packet_desc[i].status) {
			std::string	packet((char *)
				libusb_get_iso_packet_buffer(transfer, i),
				transfer->iso_packet_desc[i].actual_length);
			USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "packet size %d",
				transfer->iso_packet_desc[i].actual_length);
			packets.push_back(packet);
			packetcounter++;
		}
	}
	return packetcounter;
}	

//////////////////////////////////////////////////////////////////////
// IsoTransfer implementation
//////////////////////////////////////////////////////////////////////
IsoTransfer::IsoTransfer(EndpointDescriptorPtr _endpoint, int _totalpackets)
	: Transfer(_endpoint), totalpackets(_totalpackets),
	  lock(mutex, std::defer_lock) {
}

static void	isotransfer_event_thread(IsoTransfer *isotransferptr) {
	isotransferptr->handlevents();
}

/**
 * \brief Handle events in a separate thread
 */
void	IsoTransfer::handlevents() {
	libusb_context	*ctx = getContext();

	// wait for the mutex to unlock
	lock.lock();
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "event handling thread released");

	// the mutex was locked, so we can now start handling segments
	incoming.front()->submit();

	// wait for completion of all segments
	while (!complete) {
		int	rc = libusb_handle_events(ctx);
		if (rc != LIBUSB_SUCCESS) {
			USBdebug(LOG_ERR, DEBUG_LOG, 0, "request handling failed");
		}
		// XXX if failed, we should cancel all pending requests.
	}
}

/**
 * \brief Isochronous transfer implementation
 *
 * Doing an isochrounous transfer for a certain number of bytes is
 * quite complicated. In an isochronous transfer, a packet is transmitted
 * in every micro frame, even if there is no new data available. So the
 * transfer has to be resubmitted each time a few packets have been received.
 * \param dev_handle	libusb_device_handle to use for the transfer
 */
void	IsoTransfer::submit(libusb_device_handle *dev_handle) {
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "preparing isochronous transfer");

	// find the packet size that the endpoint can handle
	int	packetsize = endpoint->maxPacketSize()
			* endpoint->transactionOpportunities();
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "found packet size: %d", packetsize);

	// each segment should be the same number of segments
	int	packets_per_segment = 400;

	// create a bunch of IsoSegments and add them to the queue
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "total packets: %d", totalpackets);
	int	packetcount = 0;
	while (packetcount < totalpackets) {
		IsoSegment	*segptr
			= new IsoSegment(endpoint, packets_per_segment,
				this, dev_handle, timeout);
		incoming.push(IsoSegmentPtr(segptr));
		packetcount += packets_per_segment;
	}
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "incoming now contains %d segments",
		incoming.size());
	if (incoming.size() == 0) {
		return;
	}

	// mark the transfer as incomplete
	complete = false;

	// lock the mutex, this will cause the thread to block when it starts
	lock.lock();
	
	// now create a new thread which will handle the events. But because
	// the mutex is locked, it will not start working just yet, only
	// when the mutex is unlocked, that thread will be released
	try {
		eventthread = std::thread(isotransfer_event_thread, this);
	} catch (...) {
		throw USBError("cannot create event handling thread");
	}

	// wait for completion of the request, using the condition variable
	// this will release the lock, so the thread will be released too
	try {
		condition.wait(lock);
	} catch (...) {
		throw USBError("cannot release event handling thread");
	}

	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "all callbacks completed");

	// wait for the thread to terminate
	eventthread.join();

	// copy all the packets
	while (outgoing.size() > 0) {
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0,
			"extracting packets from segment");
		outgoing.front()->extract(packets);
		outgoing.pop();
	}
	USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "have now %d packets", packets.size());
}

IsoTransfer::~IsoTransfer() {
}


void	IsoTransfer::callback(libusb_transfer * /* transfer */) {
	// the front element of the incoming queue is the one we were
	// working on before the callback completed. So we have to
	// take it from the incoming queue and add it to the outgoing
	// queue.
	outgoing.push(incoming.front());
	incoming.pop();

	// now we can commit the next element of the incoming queue, if
	// there is one
	if (incoming.size() > 0) {
		incoming.front()->submit();
	} else {
		USBdebug(LOG_DEBUG, DEBUG_LOG, 0, "all segments complete");
		complete = true;
		lock.unlock();
		condition.notify_one();
	}
}

} // namespace usb
} // namespace astro
