/*
 * Memory.cpp -- Memory statistics
 *
 * (c) 2020 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#include <AstroStatistics.h>

namespace astro {
namespace statistics {

unsigned long	Memory::_number_of_image_allocations = 0;
unsigned long	Memory::_number_of_image_deallocations = 0;
unsigned long long	Memory::_bytes_allocated_for_images = 0;
unsigned long long	Memory::_bytes_allocated_for_images_total = 0;

void	Memory::image_allocate(unsigned long size) {
	_number_of_image_allocations++;
	_bytes_allocated_for_images += size;
	_bytes_allocated_for_images_total += size;
}

void	Memory::image_allocate(unsigned long pixels, unsigned int pixelsize) {
	image_allocate(pixels * pixelsize);
}

void	Memory::image_deallocate(unsigned long size) {
	_number_of_image_deallocations--;
	_bytes_allocated_for_images -= size;
}

void	Memory::image_deallocate(unsigned long pixels, unsigned int pixelsize) {
	image_deallocate(pixels * pixelsize);
}

} // namespace statistics
} // namespace astro
