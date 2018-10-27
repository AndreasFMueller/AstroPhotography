/*
 * JPEG.h -- definitions for JPEG conversions
 *
 * (c) 2018 Prof Dr Andreas Müller, Hochschule Rapperswil
 */
#ifndef _JPEG_H
#define _JPEG_H

#include <AstroImage.h>

namespace astro {
namespace image {

class JPEG {
	int	_quality;
public:
	static bool	isjpegfilename(const std::string& filename);
	JPEG();
	int	quality() const { return _quality; }
	void	quality(int q) { _quality = q; }

	// basic write operations with 8bit pixel sizes
	size_t	writeJPEG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
			void **buffer, size_t *buffersize);
	size_t	writeJPEG(const ConstImageAdapter<RGB<unsigned char> >& colorimage,
			const std::string& filename);
	size_t	writeJPEG(const ConstImageAdapter<unsigned char>& monoimage,
			void **buffer, size_t *buffersize);
	size_t	writeJPEG(const ConstImageAdapter<unsigned char>& monoimage,
			const std::string& filename);

#if 0
	// write operations for larger pixels
	template<typename Pixel>
	size_t	writeJPEG(const ConstImageAdapter<RGB<Pixel> >& colorimage,
			void **buffer, size_t *buffersize);
	template<typename Pixel>
	size_t	writeJPEG(const ConstImageAdapter<RGB<Pixel> >& colorimage,
			const std::string& filename);
	template<typename Pixel>
	size_t	writeJPEG(const ConstImageAdapter<Pixel>& monoimage,
			void **buffer, size_t *buffersize);
	template<typename Pixel>
	size_t	writeJPEG(const ConstImageAdapter<Pixel>& monoimage,
			const std::string& filename);
#endif

	// write generic image
	size_t	writeJPEG(const ImagePtr image,
			void **buffer, size_t *buffersize);
	size_t	writeJPEG(const ImagePtr image, const std::string& filename);

	// read JPEG images
	ImagePtr	readJPEG(const std::string& filename);
	ImagePtr	readJPEG(void *buffer, size_t buffersize);
};

} // namespace image
} // namespace astro

#endif /* _JPEG_H */
