/*
 * AstroStatistics.h
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _AstroStatistics_h
#define _AstroStatistics_h

#include <string>

namespace astro {
namespace statistics {

/**
 * \brief Class to encapsulate memory related statistics
 */
class Memory {
	static unsigned long		_number_of_image_allocations;
	static unsigned long		_number_of_image_deallocations;
	static unsigned long long	_bytes_allocated_for_images;
	static unsigned long long	_bytes_allocated_for_images_total;
public:
	static void	image_allocate(unsigned long size);
	static void	image_allocate(unsigned long pixels,
				unsigned int pixelsize);
	static void	image_deallocate(unsigned long size);
	static void	image_deallocate(unsigned long pixels,
				unsigned int pixelsize);

	static unsigned long	number_of_image_allocations() {
		return _number_of_image_allocations;
	}
	static unsigned long	number_of_image_deallocations() {
		return _number_of_image_deallocations;
	}
	static unsigned long long	bytes_allocated_for_images() {
		return _bytes_allocated_for_images;
	}
	static unsigned long long	bytes_allocated_for_images_total() {
		return _bytes_allocated_for_images_total;
	}
};

class Statistics {
public:
	typedef std::string	Key;
};

} // namespace statistics
} // namespace astro

#endif /* _AstroStatistics_h */
