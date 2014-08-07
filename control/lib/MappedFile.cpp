/*
 * MappedFile.cpp -- Memory mapped file implementation
 *
 * (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
 */
#include <MappedFile.h>
#include <includes.h>
#include <AstroFormat.h>
#include <AstroDebug.h>
#include <stdexcept>

namespace astro {
namespace catalog {

/**
 * \brief Construct a Mapped file
 */
MappedFile::MappedFile(const std::string& filename, size_t recordlength)
	: _recordlength(recordlength) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mapping file '%s'", filename.c_str());
	// stat the file
	struct stat	sb;
	if (stat(filename.c_str(), &sb) < 0) {
		std::string	msg = stringprintf("cannot stat '%s': %s",
			filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}
	data_len = sb.st_size;
	if (0 != (data_len % _recordlength)) {
		debug(LOG_DEBUG, DEBUG_LOG, 0, "record length %u does not "
			"divide file size %u", recordlength, data_len);
		throw std::runtime_error("record length does not devide "
			"file size");
	}
	_nrecords = data_len / _recordlength;
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file contains %u records", _nrecords);

	// open the file
	debug(LOG_DEBUG, DEBUG_LOG, 0, "open file '%s'", filename.c_str());
	int	fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0) {
		std::string	msg = stringprintf("cannot open '%s': %s",
			filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

        // map the file into the address space
	debug(LOG_DEBUG, DEBUG_LOG, 0, "mapping '%s', length %u",
		filename.c_str(), data_len);
	data_ptr = (char *)mmap(NULL, sb.st_size, PROT_READ,
		MAP_FILE | MAP_PRIVATE, fd, 0);
	if ((void *)(-1) == data_ptr) {
		close(fd);
		std::string     msg = stringprintf("cannot map '%s': %s",
			filename.c_str(), strerror(errno));
		debug(LOG_DEBUG, DEBUG_LOG, 0, "%s", msg.c_str());
		throw std::runtime_error(msg);
	}

        // we can now close the file descriptor
        close(fd);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "file '%s' mapped", filename.c_str());
}

/**
 * \brief Unmap the file from the address space
 */
MappedFile::~MappedFile() {
	if (NULL != data_ptr) {
		munmap(data_ptr, data_len);
	}
}

/**
 * \brief retrieve a specific record
 */
std::string	MappedFile::get(size_t record_number) const {
	if (record_number >= _nrecords) {
		throw std::runtime_error("record number too large");
	}
	//debug(LOG_DEBUG, DEBUG_LOG, 0, "retrieving record %u", record_number);
	return std::string(&((char *)data_ptr)[_recordlength * record_number],
		_recordlength);
}

} // namespace catalog
} // namespace astro
