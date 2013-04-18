/*
 * USBIsoTransfer.cpp
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <AstroUSB.h>
#include <debug.h>

namespace astro {
namespace usb {

//////////////////////////////////////////////////////////////////////
// IsoPacket implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Construct an IsoPacket from the data
 */
IsoPacket::IsoPacket(unsigned char *data, size_t length, int _status)
	: std::string((char *)data, length), status(_status) {
}

//////////////////////////////////////////////////////////////////////
// IsoSegment implementation
//////////////////////////////////////////////////////////////////////

static void isotransfer_callback(libusb_transfer *transfer) {
	IsoTransfer	*isotransferptr = (IsoTransfer *)transfer->user_data;
	isotransferptr->callback();
}

/**
 * \brief Create an isochronous segment.
 *
 */
IsoSegment::IsoSegment(EndpointDescriptorPtr _endpoint, int packets,
	IsoTransfer *_isotransfer, libusb_device_handle *dev_handle,
	int timeout)
	throw(USBError) : endpoint(_endpoint), isotransfer(_isotransfer) {
	// compute the packet size for the packets that we need
	int	packetsize = endpoint->maxPacketSize()
			* endpoint->transactionOpportunities();

	// allocate the transfer
	transfer = libusb_alloc_transfer(packets);
	if (NULL == transfer) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot allocate transfer");
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
	debug(LOG_DEBUG, DEBUG_LOG, 0, "created IsoSegment with %d packets",
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
void	IsoSegment::submit() throw(USBError) {
	libusb_submit_transfer(transfer);
}

int	IsoSegment::extract(std::list<IsoPacketPtr>& packets) {
	int	packetcounter = 0;
	for (unsigned int i = 0; i < transfer->num_iso_packets; i++) {
		IsoPacketPtr	packetptr(new IsoPacket(
			libusb_get_iso_packet_buffer(transfer, i),
			transfer->iso_packet_desc[i].actual_length,
			transfer->iso_packet_desc[i].status));
		packets.push_back(packetptr);
		packetcounter++;
	}
	return packetcounter;
}	

//////////////////////////////////////////////////////////////////////
// IsoTransfer implementation
//////////////////////////////////////////////////////////////////////
IsoTransfer::IsoTransfer(EndpointDescriptorPtr _endpoint, int _totalpackets)
	: Transfer(_endpoint, 0), totalpackets(_totalpackets) {
}

static void	*isotransfer_event_thread(void *arguments) {
	IsoTransfer	*isotransferptr = (IsoTransfer *)arguments;
	isotransferptr->handlevents();
	return NULL;
}

/**
 * \brief Handle events in a separate thread
 */
void	IsoTransfer::handlevents() {
	libusb_context	*ctx
		= endpoint->device().getContext()->getLibusbContext();

	// wait for the mutex to unlock
	if (pthread_mutex_lock(&mutex)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "event thread cannot lock mutex");
		return;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "event handling thread released");

	// the mutex was locked, so we can now start handling segments
	incoming.front()->submit();

	// wait for completion of all segments
	while (!complete) {
		int	rc = libusb_handle_events(ctx);
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
void	IsoTransfer::submit(libusb_device_handle *dev_handle) throw(USBError) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "preparing isochronous transfer");

	// find the packet size that the endpoint can handle
	int	packetsize = endpoint->maxPacketSize()
			* endpoint->transactionOpportunities();
	debug(LOG_DEBUG, DEBUG_LOG, 0, "found packet size: %d", packetsize);

	// each segment should be the same number of segments
	int	packets_per_segment = 400;

	// create a bunch of IsoSegments and add them to the queue
	debug(LOG_DEBUG, DEBUG_LOG, 0, "total packets: %d", totalpackets);
	int	packetcount = 0;
	while (packetcount < totalpackets) {
		IsoSegment	*segptr
			= new IsoSegment(endpoint, packets_per_segment,
				this, dev_handle, timeout);
		incoming.push(IsoSegmentPtr(segptr));
		packetcount += packets_per_segment;
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "incoming now contains %d segments",
		incoming.size());
	if (incoming.size() == 0) {
		return;
	}

	// mark the transfer as incomplete
	complete = false;

	// create condition variable and mutex
	pthread_cond_init(&condition, NULL);
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_lock(&mutex); // this will block the thread
	
	// now create a new thread which will handle the events. But because
	// the mutex is locked, it will not start working just yet, only
	// when the mutex is unlocked, that thread will be released
	pthread_attr_t	attrs;
	pthread_attr_init(&attrs);
	int	rc = pthread_create(&eventthread, &attrs,
		isotransfer_event_thread, this);
	if (rc) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot start thread: %s",
			strerror(rc));
		throw USBError("cannot create event handling thread");
	}

	// wait for completion of the request, using the condition variable
	if (rc = pthread_cond_wait(&condition, &mutex)) {
		debug(LOG_ERR, DEBUG_LOG, 0, "cannot cond wait: %s",
			strerror(rc));
		throw USBError("cannot release event handling thread");
	}

	// clean up condition variable and mutex
	pthread_cond_destroy(&condition);
	pthread_mutex_destroy(&mutex);

	debug(LOG_DEBUG, DEBUG_LOG, 0, "all callbacks completed");

	// copy all the packets
	while (outgoing.size() > 0) {
		outgoing.front()->extract(packets);
		outgoing.pop();
	}
	debug(LOG_DEBUG, DEBUG_LOG, 0, "have now %d packets", packets.size());
}

IsoTransfer::~IsoTransfer() {
}


void	IsoTransfer::callback() {
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
		debug(LOG_DEBUG, DEBUG_LOG, 0, "all segments complete");
		complete = true;
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&condition);
	}
}

} // namespace usb
} // namespace astro
