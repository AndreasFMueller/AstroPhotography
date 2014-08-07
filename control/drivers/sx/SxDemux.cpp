/*
 * SxDemux.cpp -- demultiplexing stuff
 *
 * (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperwil
 */
#include <SxDemux.h>
#include <AstroDebug.h>

namespace astro {
namespace camera {
namespace sx {

//////////////////////////////////////////////////////////////////////
// Field implementation
//////////////////////////////////////////////////////////////////////

/**
 * \brief Create a field object.
 *
 * \param _size	size of the image object of which this field is a part
 * \param l	length of the data block (should be size.with * size.height / 2)
 */
Field::Field(ImageSize _size, size_t l) : size(_size), length(l) {
	if (size.width() * size.height() != 2 * l) {
		debug(LOG_ERR, DEBUG_LOG, 0, "%dx%d image expects length %d, "
			"%d found", size.width(), size.height(),
			size.width() * size.height() / 2, length);
		throw std::logic_error("image size and field size mismatch");
	}
	data = new unsigned short[length];
}

/**
 * \brief destroy the field object.
 */
Field::~Field() {
	delete [] data;
	data = NULL;
}

/**
 * \brief Rescale a field
 *
 * This method scales the pixels of the field with the factor. The factor
 * must be >1 because otherwise saturated pixels become unsaturated by
 * the scaling operation, leading to wrong colors.
 * \param field		field object to rescale
 * \param scale
 */
void	Field::rescale(double scale) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "rescale field by factor %f", scale);
	for (size_t i = 0; i < length; i++) {
		unsigned long	rescaled = data[i] * scale;
		if (rescaled > 0xffff) {
			data[i] = 0xffff;
		} else {
			data[i] = rescaled;
		}
	}
}

/**
 * \brief Output of fields (mainly for testing)
 *
 * 
 */
std::ostream&	operator<<(std::ostream& out, const Field& field) {
	debug(LOG_DEBUG, DEBUG_LOG, 0, "writing length %d field from "
		"%d x %d image", field.length,
		field.size.width(), field.size.height());
	unsigned int	width = field.size.width();
	out.write((const char *)&width, sizeof(width));
	unsigned int	height = field.size.height();
	out.write((const char *)&height, sizeof(height));
	out.write((const char *)&field.length, sizeof(field.length));
	out.write((const char *)field.data, 2 * field.length);
	return out;
}

/**
 * \brief Input of fields (mainly for testing)
 */
std::istream&	operator>>(std::istream& in, Field& field) {
	unsigned int	width;
	in.read((char *)&width, sizeof(width));
	field.size.setWidth(width);
	unsigned int	height;
	in.read((char *)&height, sizeof(height));
	field.size.setHeight(height);
	in.read((char *)&field.length, sizeof(field.length));
	debug(LOG_DEBUG, DEBUG_LOG, 0, "reading length %d field", field.length);
	delete [] field.data;
	field.data = new unsigned short[field.length];
	in.read((char *)field.data, 2 * field.length);
	return in;
}

//////////////////////////////////////////////////////////////////////
// Demuxer base class implementation
//////////////////////////////////////////////////////////////////////

Demuxer::Demuxer() {
}

Demuxer::~Demuxer() {
}

void	Demuxer::set_pixel(Image<unsigned short>& image, int x, int y,
		unsigned short value) const {
	if (x < 0) {
		return;
	}
	if (x >= width) {
		return;
	}
	if (y < 0) {
		return;
	}
	if (y >= height) {
		return;
	}
	image.pixel(x, y) = value;
}

void	Demuxer::operator()(Image<unsigned short>& image,
			const Field& /* field1 */, const Field& /* field2 */) {
	width = image.size().width();
	height = image.size().height();
}

void	Demuxer::PIXEL(Image<unsigned short>& image, int x, int y,
		unsigned short v) {
	set_pixel(image, x, y, v);
}

void	Demuxer::LEXIP(Image<unsigned short>& image, int x, int y,
		unsigned short v) {
	set_pixel(image, width - x, y, v);
}

void	Demuxer::set_quad(Image<unsigned short>& image, int x, int y,
		const Field& field, int off) {
	int     p;
	p = perm[0] << 1; PIXEL(image, x + 0, y + 0, field.data[off + p]);
	p = perm[1] << 1; PIXEL(image, x + 2, y + 0, field.data[off + p]);
	p = perm[2] << 1; PIXEL(image, x + 0, y + 2, field.data[off + p]);
	p = perm[3] << 1; PIXEL(image, x + 2, y + 2, field.data[off + p]);
}

void	Demuxer::set_quad_back(Image<unsigned short>& image,
		int x, int y, const Field& field, int off) {
	int     p;
	p = permb[0] << 1; LEXIP(image, x + 0, y + 0, field.data[off + p]);
	p = permb[1] << 1; LEXIP(image, x + 2, y + 0, field.data[off + p]);
	p = permb[2] << 1; LEXIP(image, x + 0, y + 2, field.data[off + p]);
	p = permb[3] << 1; LEXIP(image, x + 2, y + 2, field.data[off + p]);
}

//////////////////////////////////////////////////////////////////////
// DemuxerBinned base class implementation
//////////////////////////////////////////////////////////////////////
DemuxerBinned::DemuxerBinned() {
	offset = 0;
	perm[0] = 1;
	perm[1] = 0;
	perm[2] = 3;
	perm[3] = 2;
	permb[0] = 0;
	permb[1] = 1;
	permb[2] = 2;
	permb[3] = 3;
	greenx = 0; greeny = 0;
	redx = 0; redy = 0;
	bluex = 0; bluey = 0;
}

DemuxerBinned::~DemuxerBinned() {
}

void	DemuxerBinned::operator()(Image<unsigned short>& image,
		const Field& field1, const Field& field2) {
	Demuxer::operator()(image, field1, field2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "binned demultiplexer, offset = %d",
		offset);

	int	off = 2 * offset + 1;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad_back(image,
				x + 1 + bluex - 2, y + 1 + bluey,
				field1, off);
			off += 8;
		}
	}

	off = 2 * offset;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad(image,
				x + 1, y + 0,
				field1, off);
			off += 8;
		}
	}

	off = 2 * offset + 1;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad_back(image,
				x + redx - 2, y + redy,
				field2, off);
			off += 8;
		}
	}

	off = 2 * offset;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad(image,
				x + 0 + greenx + 2,
				y + 1 + greeny,
				field2, off);
			off += 8;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// DemuxerUnbinned base class implementation
//////////////////////////////////////////////////////////////////////

DemuxerUnbinned::DemuxerUnbinned() {
	offset = 0;
	perm[0] = permb[0] = 0;
	perm[1] = permb[1] = 1;
	perm[2] = permb[2] = 2;
	perm[3] = permb[3] = 3;
	greenx = 0; greeny = 0;
	redx = 0; redy = 0;
	bluex = 0; bluey = 0;
}

DemuxerUnbinned::~DemuxerUnbinned() {
}

#define GREENSHIFTX     1
#define GREENSHIFTY     -1
#define BLUESHIFTX      0
#define BLUESHIFTY      0
#define GBSHIFTX        0
#define GBSHIFTY        0
#define REDSHIFTX       0
#define REDSHIFTY       0
#define GRSHIFTX        0
#define GRSHIFTY        0

void	DemuxerUnbinned::operator()(Image<unsigned short>& image,
		const Field& field1, const Field& field2) {
	Demuxer::operator()(image, field1, field2);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "unbinned demultiplexer, offset = %d",
		offset);

#if 1
	int	off = 2 * offset + 1;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad_back(image,
				x + 1 + BLUESHIFTX, y + 1 + BLUESHIFTY,
				field1, off);
			off += 8;
		}
	}

	off = 2 * offset + 2;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad(image,
				x + 1 + GBSHIFTX + GREENSHIFTX,
				y + 0 + GBSHIFTY + GREENSHIFTY,
				field1, off);
			off += 8;
		}
	}

	off = 2 * offset + 1;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad_back(image,
				x + REDSHIFTX, y + REDSHIFTY,
				field2, off);
			off += 8;
		}
	}

	off = 2 * offset + 2;
	for (int x = 0; x < width; x += 4) {
		for (int y = 0; y < height; y += 4) {
			set_quad(image,
				x + 0 + GRSHIFTX + GREENSHIFTX,
				y + 1 + GRSHIFTY + GREENSHIFTY,
				field2, off);
			off += 8;
		}
	}
#else
	/* copy without demultiplexing, just for debugging */
	unsigned int	length = field1.getLength() * sizeof(unsigned short);
	debug(LOG_DEBUG, DEBUG_LOG, 0, "length: %u", length);
	memcpy(image.pixels, field1.data, length);
	memcpy(length + (unsigned char *)image.pixels, field2.data, length);
#endif
}

} // namespace sx
} // namespace camera
} // namespace astro

